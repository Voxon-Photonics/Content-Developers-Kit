'''
Voxon X Blender Add-On by Holophant Studios
By Ryan Silverman
Adapted from Ben Weatherall's Voxon Plugin
'''

bl_info = {
	"name": "Voxon X Blender Add-on",
	"version": (2, 4, 2),
	"author": "Holophant Studios",
	"blender": (3, 0, 0),
	"category": "Render",
}

from bpy.types import Panel, Operator, PropertyGroup, AddonPreferences

from bpy.props import StringProperty, BoolProperty, PointerProperty, EnumProperty, FloatVectorProperty, IntProperty

import bpy, mathutils
import os
import sys
import importlib
import importlib.util
import site
from bpy.app.handlers import persistent

import time, functools
import struct
import traceback

# Needed to use crc32c
def ensure_site_packages(packages: list, directory: str):
    if not packages:
        return

    os.makedirs(directory, exist_ok = True)

    if not directory in sys.path:
        sys.path.append(directory)

    modules_to_install = [module[1] for module in packages if not importlib.util.find_spec(module[0])]
    if not modules_to_install:
        return

    python_binary = sys.executable

    import subprocess
    subprocess.run([python_binary, '-m', 'ensurepip'], check=True)
    subprocess.run([python_binary, '-m', 'pip', 'install', *modules_to_install, "--target", directory], check=True)

ensure_site_packages([("crc32c", "crc32c")], directory = site.getusersitepackages())

from . import bigpak
importlib.reload(bigpak) # Update the cached files when loading a new version of the add-on
from . import voxieplay
importlib.reload(voxieplay)
from . import voxierecording
importlib.reload(voxierecording)
from . import addon_updater_ops
importlib.reload(addon_updater_ops)
from . import addon_updater
importlib.reload(addon_updater)

from .bigpak import BigPak
from .voxieplay import *
from .voxierecording import make_recording

# Start of actual code
CONNECTION = None
REGISTERED_LOOP = None
	
# Function called for Voxon Rendering
class VoxonCast():
	error = ''
	def __init__(self, HOST, PORT = 8080):
		self.host = HOST
		self.port = PORT
		self.msg = CmdRequestInfo()
		self.start_time = time.time()
		self.remote_time = 0.0
	
	@staticmethod
	def error_window(self, context):
		error_lines = VoxonCast.error.split('\n')
		if not len(error_lines[-1]):
			error_lines.pop()
		for text in error_lines: 
			row = self.layout.row(align = True)
			row.alignment = 'EXPAND'
			row.label(text=text)
			row = self.layout.row(align = True)
	
	@staticmethod
	def show_error():
		VoxonCast.error = traceback.format_exc()
		print(VoxonCast.error)
		bpy.context.window_manager.popup_menu(VoxonCast.error_window, title = 'Voxon Error', icon = 'ERROR')
	
	@staticmethod
	def show_custom_error(err):
		VoxonCast.error = err
		print(VoxonCast.error)
		bpy.context.window_manager.popup_menu(VoxonCast.error_window, title = 'Voxon Error', icon = 'ERROR')

	
	def connect(self):
		try:
			if(self.bigpak):
				self.bigpak.close()
		except Exception:
			pass
		try:
			self.bigpak = BigPak(self.host, self.port, recv_port = 8090)
			self.image_name_num = 0 # how image names are made
			for mat in bpy.data.images:
				mat.pop('image_name', None) # make sure images are reloaded
			self.old = False
			if bpy.app.version < (3, 5, 0):
				self.old = True
			
			# For backwards compatability with older add-on versions
			scene = bpy.context.scene
			if ('VIEWBOX' not in scene) and ('VIEWBOX' in scene.objects):
				scene['VIEWBOX'] = scene.objects['VIEWBOX']
			return True
		except Exception:
			VoxonCast.show_error()
			return False
		
	def disconnect(self):
		print("Disconnecting from host")
		if(self.bigpak):
			try:
				self.bigpak.close()
			except Exception:
				print("Failed to disconnect")
				pass # avoid being stuck if port was already disconnected
		for obj in bpy.data.objects:
			# This is done to keep the file size small
			obj.pop('verts', None)
			obj.pop('cols', None)
			obj.pop('uvs', None)
			obj.pop('meshes', None)
		bpy.context.preferences.addons[__name__].preferences.connected = False
			
	def loop(self):
		if(self.bigpak == None):
			return 0.1
			
		if((time.time() - self.start_time) < self.remote_time):
			return 0.01

		dps = bpy.context.evaluated_depsgraph_get()
		scene = bpy.context.scene
		voxon_props = scene.voxon_properties
		
		# Start Frame
		self.msg += CmdFrameStart(time.time() - self.start_time)
		# Set ViewPort
		aspect = voxon_props.aspect
		self.msg += CmdSetView(aspect)
		
		use_viewbox = 'VIEWBOX' in scene
		if use_viewbox:
			viewbox = scene['VIEWBOX']
			if viewbox is None:
				use_viewbox = False
			else:
				viewbox_location = viewbox.location
				viewbox_radius = viewbox.dimensions.magnitude/2
				matrix_viewbox = viewbox.matrix_world.inverted()
		
		if voxon_props.reload_images:
			for mat in bpy.data.images:
				mat.pop('image_name', None)
			self.image_name_num = 0
			voxon_props.reload_images = False
		# Get Objects
		for obj in scene.objects:
			if obj.type != 'MESH':
				continue
			if use_viewbox and (obj is viewbox):
				if voxon_props.show_viewbox or obj.select_get():
					aspect = voxon_props.aspect
					self.msg += CmdDrawBox((-aspect[0],-aspect[1],-aspect[2],aspect[0],aspect[1],aspect[2]),1,0xffffff)	
				continue	
			if obj.hide_render:
				continue
			
			try:
				object_eval = obj.evaluated_get(dps) # get mesh with modifiers

				if use_viewbox: # Bounding sphere intersection algorithm
					local_obj_location = 0.5 * (mathutils.Vector(object_eval.bound_box[0]).to_4d() + mathutils.Vector(object_eval.bound_box[6]).to_4d())
					obj_location = (obj.matrix_world @ local_obj_location).to_3d()
					dist = (viewbox_location - obj_location).magnitude
					if dist > (viewbox_radius + object_eval.dimensions.magnitude/2):
						continue

				mesh = object_eval.to_mesh()
				mesh.transform(obj.matrix_world)
				if use_viewbox:
					mesh.transform(matrix_viewbox)

				# Material Properties
				color = 0xFFFFFF
				image_texture = False
				vertex_color = False
				image_name = b''
				fillmode = 2
				
				if(len(mesh.materials) > 0):
					material = mesh.materials[0]
					if material.use_nodes:
						output_node = material.node_tree.get_output_node('ALL')
						if output_node != None:
							shader_node = output_node.inputs[0].links[0].from_node
							shader_node_type = shader_node.bl_idname
							if shader_node_type == 'ShaderNodeBsdfPrincipled' or shader_node_type == 'ShaderNodeBsdfDiffuse':
								if shader_node.inputs[0].is_linked:
									color_node = shader_node.inputs[0].links[0].from_node
									if color_node.bl_idname == 'ShaderNodeTexImage':
										image = color_node.image
										if (image is not None) and len(image.filepath):
											color = 0x404040
											image_texture = True
											image_path = bpy.path.abspath(image.filepath, library=image.library) # Because Blender does paths weird
											try:
												image_mod_time = os.stat(image_path).st_mtime
											except OSError:
												VoxonCast.show_custom_error("File not found: " + image_path)
												break
											if ('image_name' not in image):
												image['image_name'] = str(self.image_name_num).encode('ascii') + b'.png'
												self.image_name_num += 1
												image['image_path'] = image_path
												image['image_mod_time'] = image_mod_time
												self.msg += CmdSendFile(image_path, image['image_name'])
											elif  (image_path != image['image_path']) or (image_mod_time != image['image_mod_time']):
												image['image_path'] = image_path
												image['image_mod_time'] = image_mod_time
												self.msg += CmdSendFile(image_path, image['image_name'])
											image_name = image['image_name']
										else:
											continue
									elif color_node.bl_idname == 'ShaderNodeVertexColor':
										vertex_color = True
									else:
										continue
								else:
									tri_color = shader_node.inputs[0].default_value
									color = int(tri_color[2] * 255)
									color += int(tri_color[1] * 255) << 8
									color += int(tri_color[0] * 255) << 16
							else:
								continue
						else:
							continue
					else:
						tri_color = material.diffuse_color
						color = int(tri_color[2] * 255)
						color += int(tri_color[1] * 255) << 8
						color += int(tri_color[0] * 255) << 16

					fillmode = int(mesh.materials[0].fillmode)
					if vertex_color:
						fillmode += 16
				if color == 0:
					continue # do not render things that are black
				bl_verts = mesh.vertices
				verts = []
				for bl_vert in bl_verts:
					verts.append(-bl_vert.co[0])
					verts.append(bl_vert.co[1])
					verts.append(-bl_vert.co[2])

				if 'verts' not in obj:
					obj['use_image'] = False
					obj['use_cols'] = False
					obj['recalc'] = True
					obj['edge_count'] = -1
				obj['verts'] = verts

				vert_bits = (len(bl_verts) - 1).bit_length()
				uvc_bits = 0

				if image_texture:
					if not obj['use_image']:
						obj['use_image'] = True
						obj['recalc'] = True
					if self.old:
						bl_uvs = mesh.uv_layers.active.data
						uvs = []
						for bl_uv in bl_uvs:
							uvs.append(bl_uv.uv[0])
							uvs.append(1 - bl_uv.uv[1])
					else:
						bl_uvs = mesh.uv_layers.active.uv
						uvs = []
						for bl_uv in bl_uvs:
							uvs.append(bl_uv.vector[0])
							uvs.append(1 - bl_uv.vector[1])
					obj['uvs'] = uvs
					uvc_bits = (len(bl_uvs) - 1).bit_length()
				elif obj['use_image']:
					obj['use_image'] = False
					obj['recalc'] = True
	
				if vertex_color:
					if not obj['use_cols']:
						obj['use_cols'] = True
						obj['recalc'] = True
					bl_cols_loc = mesh.color_attributes.active_color
					if bl_cols_loc is None:
						continue
					bl_cols = bl_cols_loc.data
					cols = []
					for bl_col in bl_cols:
						tri_col = bl_col.color
						col = int(tri_col[2] * 255)
						col += int(tri_col[1] * 255) << 8
						col += int(tri_col[0] * 255) << 16
						cols.append(col)
					obj['cols'] = cols
					uvc_bits = (len(cols) - 1).bit_length()

					if 'triangulated' not in obj or not obj['triangulated']:
						obj['triangulated'] = False
						for mod in obj.modifiers:
							if mod.type == 'TRIANGULATE':
								obj['triangulated'] = True
								break
						if not obj['triangulated']:
							obj.modifiers.new("Triangulate", "TRIANGULATE")
							obj['triangulated'] = True
				elif obj['use_cols']:
					obj['use_cols'] = False
					obj['recalc'] = True

				
				test_loop = mesh.loops[len(mesh.loops)//2].vertex_index
				if obj['edge_count'] != len(mesh.edges) or 'test_loop' not in obj or obj['test_loop'] != test_loop:
					obj['edge_count'] = len(mesh.edges)
					obj['recalc'] = True
				
				if obj['recalc']:
					index_bytes = (vert_bits + uvc_bits + 8)//8 # +7 makes it ceil, +1 for start_loop bit
					start_loop = 1 << (vert_bits + uvc_bits)

					meshes = b''
					polys = mesh.polygons
					if image_texture or vertex_color:
						for poly in polys:
							loop_total = poly.loop_total

							loop_index = poly.loop_start
							meshes += (start_loop + (loop_index << vert_bits) + mesh.loops[loop_index].vertex_index).to_bytes(index_bytes, 'little')
							# uvc = loop_index, so meshes = 1 uvc xyz

							for loop_index in range(poly.loop_start + 1, poly.loop_start + loop_total):
								meshes += ((loop_index << vert_bits) + mesh.loops[loop_index].vertex_index).to_bytes(index_bytes, 'little')
					else:
						for poly in polys:
							loop_total = poly.loop_total

							loop_index = poly.loop_start
							meshes += (start_loop + mesh.loops[loop_index].vertex_index).to_bytes(index_bytes, 'little')
							# uvc = loop_index, so meshes = 1 xyz

							for loop_index in range(poly.loop_start + 1, poly.loop_start + loop_total):
								meshes += (mesh.loops[loop_index].vertex_index).to_bytes(index_bytes, 'little')

					obj['meshes'] = meshes
						
				self.msg += CmdDrawMeshTex(obj, fillmode, color, image_name)
				obj['recalc'] = False
				obj['test_loop'] = test_loop

				# object_eval.to_mesh_clear()
			except Exception:
				obj['recalc'] = True
				VoxonCast.show_error()
				self.msg = b''
				self.disconnect()

				return None
		
		self.msg += CmdFrameEnd()

		# Get Response')
		try:
			self.bigpak.send_msg(self.msg)
			self.msg = b''
			
			self.ProcessResponse()
		except BigPak.error as e:
			if not (hasattr(e, 'winerror') and e.winerror == 10054):
				if e.errno == 65:
					VoxonCast.show_custom_error("Unable to find device with specified IP address")
				else:
					VoxonCast.show_error()
				self.disconnect()
				self.bigpak = None
				return None
		except Exception:
			VoxonCast.show_error()
			self.disconnect()
			self.bigpak = None
			return None
			
		# print("Process time: {}".format((time.process_time()-timer_start)*1000))
		return 0.01

	# Primarily to get aspect ratio info	
	def ProcessResponse(self):
		gen_viewbox = bpy.context.scene.voxon_properties.gen_viewbox
		if gen_viewbox:
			try:
				response = self.bigpak.recv_msg()
			except BigPak.error as e:
				if e.errno != 11 and e.errno != 35:
					if hasattr(e, 'winerror') and e.winerror == 10035:
						pass
					else:
						raise e
				return
			# Process Data
			response_len = len(response)
			
			try:
				if response_len >= 4 and struct.unpack('I', response[0:4])[0] == 0x6c527443:
					i = 4
					while(i < response_len):
						cmd = struct.unpack('B', response[i:i + 1])[0]
						if cmd == 0:
							break
						elif cmd < 8: # Mouse movement
							i += cmd.bit_count()*2 + 1
						elif cmd == 15: # Xbox controller
							i += 2
						elif cmd <= 16 and cmd < 48: # Xbox controller
							i += 3
						elif cmd >= 48 and cmd < 79: # Space navigator
							if (cmd & 7) < 6:
								i += 3
							else:
								i += 2
						elif cmd >= 128 and cmd < 144: # Keyboard
							i += cmd - 126 # - 128 + 2
						elif cmd >= 144 and cmd < 160: # Keyboard
							i += 3*(cmd - 143) + 1
						elif cmd == 160: # Info packet
							i += 1
							aspect = tuple(struct.unpack('fff', response[i:i+12]))
							i += 12
							bpy.context.scene.voxon_properties.aspect = aspect
							print("Aspect ratio: "+ str(aspect))
							make_viewbox(aspect)
							bpy.context.scene.voxon_properties.gen_viewbox = False
							break
						elif cmd > 240 and cmd < 248: # Mouse buttons
							i += 1
						else:
							break
			except Exception:
				print(b'Invalid response recieved: ' + response)

def make_viewbox(aspect):
	if bpy.context.object is not None:
		bpy.ops.object.mode_set(mode='OBJECT')
	bpy.ops.object.select_all(action='DESELECT')
	scene = bpy.context.scene

	make_new = True
	if ('VIEWBOX' in scene) and (scene['VIEWBOX'] is not None):
		viewbox = scene['VIEWBOX']
		if viewbox.name in scene.objects:
			bpy.context.view_layer.objects.active = viewbox
			viewbox.select_set(True)

			# Get rid of any transformations and reapply at the end
			matrix_viewbox = viewbox.matrix_world
			matrix_viewbox_copy = matrix_viewbox.copy()
			matrix_viewbox.identity()

			bpy.ops.object.mode_set(mode='EDIT')
			bpy.ops.mesh.primitive_cube_add(scale = aspect)
			bpy.ops.mesh.select_all(action='INVERT')
			bpy.ops.mesh.delete()
			bpy.ops.object.mode_set(mode='OBJECT')
			bpy.ops.object.origin_set(type='GEOMETRY_ORIGIN')
			
			viewbox.matrix_world = matrix_viewbox_copy
			make_new = False
		else:
			scene['VIEWBOX'] = None
			if not viewbox.users:
				bpy.data.objects.remove(viewbox)

	if make_new:
		bpy.ops.mesh.primitive_cube_add(scale = aspect)
		viewbox = bpy.context.selected_objects[0]
		viewbox.show_axis = True
		viewbox.display_type = 'WIRE'
		viewbox.name = 'VIEWBOX'
		scene['VIEWBOX'] = viewbox


class VoxonRenderingPanel(Panel):
	"""Creates a Panel in the scene context of the properties editor"""
	bl_label = "Voxon Rendering"
	bl_idname = "RENDER_PT_layout"
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_context = "render"

	def draw(self, context):
		layout = self.layout
		addon_updater_ops.check_for_update_background()
		addon_updater_ops.update_notice_box_ui(self, context)

		voxon_prop = context.scene.voxon_properties
		conn_prop = context.preferences.addons[__name__].preferences

		row = layout.row()
		row.label(text='Real-Time Rendering')
		row = layout.row()
		row.prop(conn_prop, "ip_address")
		row = layout.row()
		row.prop(voxon_prop, "gen_viewbox")
		row = layout.row()
		row.prop(voxon_prop, "show_viewbox")

		row = layout.row(align=True)
		sub = row.row()
		
		if(conn_prop.connected):
			sub.operator("voxon.active_connection", text="Disconnect")
		else:
			sub.operator("voxon.active_connection", text="Connect")
		
		row = layout.row(align=True)
		sub = row.row()
		sub.operator("voxon.reload_images", text="Reload Image Textures")
		
		row = layout.separator()
		row = layout.row()
		row.label(text='Generate Viewbox')
		row = layout.row()
		row.prop(voxon_prop, "aspect")
		row = layout.row(align=True)
		sub = row.row()
		sub.operator("voxon.viewbox", text="Generate Viewbox")

		row = layout.separator()
		row = layout.row()
		row.label(text='Make Recording')
		row = layout.row()
		row.scale_y = 0.7
		row.label(text="Recording Location:")
		row = layout.row()
		row.prop(voxon_prop, "recording_path", text = "")
		if voxon_prop.recording_action == 'CREATE':
			row = layout.row()
			row.scale_y = 0.7
			row.label(text="Recording Name:")
			row = layout.row()
			row.prop(voxon_prop, "recording_name", text = "")
		row = layout.row()
		row.prop(voxon_prop, "recording_action")
		if len(voxon_prop.recording_output):
			row = layout.row()
			row.label(text=voxon_prop.recording_output, icon = 'ERROR')
		row = layout.row(align=True)
		sub = row.row()
		
		sub.operator("voxon.record", text="Make Recording")

class ViewboxWrapper(Operator):
	bl_idname = "voxon.viewbox"
	bl_label = "Make Viewbox"
	bl_description = "Generate viewbox to be used as 3D camera"

	def execute(self, context):
		aspect = bpy.context.scene.voxon_properties.aspect
		make_viewbox(aspect)
		return {'FINISHED'}

class NetworkConnectionDialog(Operator):
	bl_idname = "voxon.active_connection"
	bl_label = "VX Hardware Address"
	bl_description = "Render Blender scene on Voxon device"
	
	# This is the method that is called when the ok button is pressed
	# which is what calls the AddCube() method 
	def execute(self, context):
		global CONNECTION
		global REGISTERED_LOOP
		
		conn_prop = context.preferences.addons[__name__].preferences 
		if(conn_prop.connected):
			if(CONNECTION):
				CONNECTION.disconnect()
			if(bpy.app.timers.is_registered(REGISTERED_LOOP)):
				bpy.app.timers.unregister(REGISTERED_LOOP)
			conn_prop.connected = False
			return {'FINISHED'}
		else:
			ip_address = context.preferences.addons[__name__].preferences.ip_address
			self.report({'INFO'}, "Connecting to "+ ip_address)
			address_split = ip_address.split(":")
			ip = address_split[0]
			if len(address_split) > 1:
				port = int(address_split[1])
				CONNECTION = VoxonCast(ip, port)
			else:
				CONNECTION = VoxonCast(ip)
			conn_prop.connected = CONNECTION.connect()			
			if(conn_prop.connected):
				REGISTERED_LOOP = functools.partial(CONNECTION.loop)
				bpy.app.timers.register(REGISTERED_LOOP)
				print("Is Loop registered? {}".format(bpy.app.timers.is_registered(REGISTERED_LOOP)))
			return {'FINISHED'}

class ReloadImagesTextures(Operator):
	bl_idname = "voxon.reload_images"
	bl_label = "Reload Image Textures"

	def execute(self, context):
		context.scene.voxon_properties.reload_images = True
		self.report({'INFO'}, "Reloading image textures...")
		return {'FINISHED'}

class RecordingWrapper(Operator):
	bl_idname = "voxon.record"
	bl_label = "Make Recording"

	def execute(self, context):
		old = False
		if bpy.app.version < (3, 5, 0):
			old = True
		props = context.scene.voxon_properties
		props.recording_output = ""
		response = make_recording(props.recording_action, props.recording_path, props.recording_name, old)
		self.report(response[0], response[1])
		if response[0] == {'ERROR'}:
			props.recording_output = "Last recording failed"
		return {'FINISHED'}

class AddonPrefs(AddonPreferences):
	bl_idname = __name__
	ip_address: StringProperty(
		name = "IP Address",
		description="IP of Voxon Hardware",
		default = "127.0.0.1"
	)
	connected: BoolProperty(
		name = "Connected",
		default = False
	)

	auto_check_update: BoolProperty(
		name="Auto-check for Update",
		description="If enabled, auto-check for updates using an interval",
		default=True)

	updater_interval_months: IntProperty(
		name='Months',
		description="Number of months between checking for updates",
		default=0,
		min=0)

	updater_interval_days: IntProperty(
		name='Days',
		description="Number of days between checking for updates",
		default=1,
		min=0,
		max=31)

	updater_interval_hours: IntProperty(
		name='Hours',
		description="Number of hours between checking for updates",
		default=0,
		min=0,
		max=23)

	updater_interval_minutes: IntProperty(
		name='Minutes',
		description="Number of minutes between checking for updates",
		default=0,
		min=0,
		max=59)

	def draw(self, context):
		layout = self.layout
		addon_updater_ops.update_settings_ui(self, context)


class VoxonProperties(PropertyGroup):
	gen_viewbox: BoolProperty(
		name = "Auto-detect Viewbox",
		description = "Detect the aspect ratio of the connected Voxon device",
		default = True
	)
	reload_images: BoolProperty(
		name = "Reload Image Textures",
		default = False
	)
	show_viewbox: BoolProperty(
		name = "Render Viewbox",
		default = False
	)
	aspect: FloatVectorProperty(
		name = "Aspect Ratio",
		size = 3,
		default = (1.0,1.0,0.4)
	)
	recording_path: StringProperty(
        name= "Recording Location",
        description="Path to where recording will be stored",
        default= "",
        subtype='DIR_PATH'
	)
	recording_name: StringProperty(
		name = "Recording Name",
		description = "Name of the recording, no file extension needed",
		default= "New Voxon Recording"
	)
	recording_action: EnumProperty(
		name = "Recording Action",
		description = "",
		default = 'CREATE',
		items = [
			('CREATE', "Create", ""),
			('REPLACE', "Replace", ""),
			('APPEND', "Append", "")
		]
	)
	recording_output: StringProperty(
		name = "Recording Output",
		description = "",
		default = ""
	)
	

class Fillmode_Panel(Panel):
	bl_idname = "PANEL_PT_fillmode"
	bl_space_type = 'PROPERTIES'
	bl_region_type = 'WINDOW'
	bl_label = "Voxon Fill Mode"
	bl_context = "material"

	def draw(self, context):
		_material = context.material
		if _material:
			layout = self.layout
			layout.prop(_material, "fillmode")

# Blender functions
@persistent	
def load_post_handler(dummy):
	print("Disconnecting connection")
	conn_prop = bpy.context.preferences.addons[__name__].preferences 
	if(conn_prop.connected):
		global CONNECTION
		global REGISTERED_LOOP
		if(CONNECTION):
			CONNECTION.disconnect()
		if(bpy.app.timers.is_registered(REGISTERED_LOOP)):
			bpy.app.timers.unregister(REGISTERED_LOOP)
		conn_prop.connected = False
		time.sleep(0.1)
	return {'FINISHED'}

classes = (AddonPrefs, VoxonRenderingPanel, RecordingWrapper, NetworkConnectionDialog,
	   ViewboxWrapper, VoxonProperties, Fillmode_Panel, ReloadImagesTextures)

def register():
	# At top so updater can still be used if user updates to non-working version of add-on
	addon_updater_ops.register(bl_info)
	
	for cls in classes:
		#addon_updater_ops.make_annotations(cls)  # Avoid blender 2.8 warnings.
		bpy.utils.register_class(cls)
	
	try:
		bpy.types.Material.fillmode = EnumProperty(
			name = "Fill Mode",
			description = "",
			default = '2',
			items = [
				('0', "Dot", ""),
				('1', "Line", ""),
				('2', "Surface", "")
        	]
    	)
	except ValueError:
		pass # fillmode has already been set-up
	bpy.types.Scene.voxon_properties = PointerProperty(type=VoxonProperties)

	bpy.app.handlers.load_post.append(load_post_handler)

	
	
def unregister():
	for obj in bpy.data.objects:
		# This is done to keep the file size small
		obj.pop('verts', None)
		obj.pop('cols', None)
		obj.pop('uvs', None)
		obj.pop('meshes', None)

	global REGISTERED_LOOP
	if(bpy.app.timers.is_registered(REGISTERED_LOOP)):	
		bpy.app.timers.unregister(REGISTERED_LOOP)
	
	addon_updater_ops.unregister()
	
	for cls in reversed(classes):
		bpy.utils.unregister_class(cls)
	
	bpy.app.handlers.load_post.remove(load_post_handler)
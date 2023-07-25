import bpy, mathutils
import os, shutil, filecmp
import traceback
import time
import crc32c
from .voxieplay import *

def make_recording(action, location, name, old):
    start_make_time = time.perf_counter()
    append_text = "Append data: "
    loaded_images = set() # names/crc of loaded images
    anim_time = 0
    audio_num = 0
    cwd = ""
    create = True

    location = bpy.path.abspath(location)
    if not os.path.exists(location):
        return ({'ERROR'}, "Recording location does not exist")

    match action:
        case 'CREATE':
            # Make folder for recording assets
            path = os.path.join(location, name)
            path_og = path+"."
            filenum = 0
            while os.path.exists(path) and filenum <= 100:
                filenum += 1
                path = path_og + f"{filenum:03}"
            if filenum >= 100:
                return ({'ERROR'}, "Unable to give recording specified name")
            try:
                os.mkdir(path)
            except OSError:
                return ({'ERROR'}, "Unable to make recording at given location")
            
            # Change directory to recording folder, we will switch back to cwd upon return
            try:
                cwd = os.getcwd()
            except OSError:
                cwd = path # Blender does not have permisssion to cwd so we cannot get it
            os.chdir(path)
        case 'REPLACE':
            # Change directory to recording folder, we will switch back to cwd upon return
            try:
                cwd = os.getcwd()
            except OSError:
                cwd = location # Blender does not have permisssion to cwd so we cannot get it
            os.chdir(location)

            fnames = os.listdir('.')
            found_rec = False
            found_log = False
            for fname in fnames:
                if os.path.isfile(fname) and fname.endswith('.rec'):
                    name_end = len(fname) - 4
                    rec_name = fname[0:name_end]
                    found_rec = True
                    break
            
            if not found_rec:
                return ({'ERROR'}, "Location to replace is not a rec folder")
            
            for fname in fnames:
                if fname.endswith('.png') or fname.endswith('.log') or fname.endswith('.mp3') or fname.endswith('.rec'):
                    os.remove(fname)
                
        case 'APPEND':
            # Change directory to recording folder, we will switch back to cwd upon return
            try:
                cwd = os.getcwd()
            except OSError:
                cwd = location # Blender does not have permisssion to cwd so we cannot get it
            os.chdir(location)

            fnames = os.listdir('.')
            found_rec = False
            found_log = False
            rec_name = ""
            log_name = ""
            for fname in fnames:
                if os.path.isfile(fname):
                    if fname.endswith('.rec'):
                        if found_rec:
                            return ({'ERROR'}, "Found two rec files")
                        name_end = len(fname) - 4
                        rec_name = fname[0:name_end]
                        found_rec = True
                    elif fname.endswith('.log'):
                        if found_log:
                            return ({'ERROR'}, "Found two log files")
                        log = open(fname, "r")
                        log_contents = log.read()
                        log.close()
                        try:
                            append_info_start = log_contents.rindex(append_text)
                            append_info_start += len(append_text)
                        except ValueError:
                            return ({'ERROR'}, "Unable to parse log file correctly")
                        try:
                            append_info_end = log_contents.index("\n", append_info_start)
                        except ValueError:
                            append_info_end = len(log_contents)
                        append_info = log_contents[append_info_start:append_info_end]
                        try:
                            anim_time = float(append_info)
                        except ValueError:
                            return ({'ERROR'}, "Unable to parse log file correctly")
                        name_end = len(fname) - 4
                        log_name = fname[0:name_end]
                        found_log = True
                    elif fname.endswith('.png'):
                        loaded_images.add(fname.encode('ascii'))
                    elif fname.endswith('.mp3'):
                        audio_end = len(fname) - 4
                        audio = fname[0:audio_end]
                        if audio.isdigit():
                            audio_num = max(audio_num, int(audio) + 1)
            
            if not found_rec:
                return ({'ERROR'}, "No rec file found in given location")
            if not found_log:
                return ({'ERROR'}, "No log file found in given location")
            
            if rec_name != log_name:
                return ({'ERROR'}, "rec and log file have different names")
            
            name = rec_name
            create = False
        case _:
            return({'ERROR'}, "Action not recognized")


    # Create/open rec and log files
    try:
        if create:
            rec = open(name+".rec", "wb")
        else:
            rec = open(name+".rec", "r+b") # Mode allows pointer to be written to
    except OSError:
        os.chdir(cwd)
        log.close()
        return ({'ERROR'}, "Unable to create/open rec file")
    
    try:
        log = open(name+".log", "a")
    except OSError:
        rec.close()
        os.chdir(cwd)
        return ({'ERROR'}, "Unable to create/open log file")

    # Write beginning of rec file
    if create:
        try:
            rec.write(CmdInitialise())
            log.write("INFO: Making recording...\n")
        except OSError:
            rec.close()
            log.close()
            os.chdir(cwd)
            return ({'ERROR'}, "Failed to write files")
    else:
        rec.seek(0, 2) # Seek the end of the rec file
        try:
            log.write("\nINFO: Appending to recording...\n")
        except OSError:
            rec.close()
            log.close()
            os.chdir(cwd)
            return ({'ERROR'}, "Failed to write files")
    
    # Check for viewbox
    scene = bpy.context.scene
    if 'VIEWBOX' not in scene:
        if 'VIEWBOX' in scene.objects: # Backwards compatability
            scene['VIEWBOX'] = scene.objects['VIEWBOX']
        else:
            log.write('ERROR: Viewbox not found, make one using the "Voxon Rendering" panel')
            rec.close()
            log.close()
            os.chdir(cwd)
            return ({'ERROR'}, "Viewbox not found")
    viewbox = scene['VIEWBOX']

    # Initialize material and other important information
    try:
        image_lookup = {} # convert image path to an image file name, if possible
        some_not_rendered = False
        for obj in scene.objects:
            if obj.type != 'MESH' or (obj is viewbox):
                obj['render_voxon'] = False
                #log.write("INFO: "+obj.name+" not rendered (expected not to be rendered)\n")
                continue
            else:
                obj['render_voxon'] = True
            obj['image_name'] = b''
            obj['use_image'] = False
            obj['use_cols'] = False
            mesh = obj.to_mesh() # No need to use depsgraph here because modifiers only affect geometry
            if(len(mesh.materials) > 0):
                material = mesh.materials[0]
                obj['fillmode'] = int(mesh.materials[0].fillmode)
                
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
                                        image_path = bpy.path.abspath(image.filepath, library=image.library) # Because Blender does paths weird
                                        if image_path in image_lookup:
                                            obj['image_name'] = image_lookup[image_path]
                                            obj['voxon_material'] = 'IMAGE'
                                            obj['use_image'] = True
                                        else:
                                            try:
                                                file = open(image_path, "rb")
                                                crc = crc32c.crc32c(file.read())
                                                file.close()
                                                matched_image = False
                                                id_num = 0 # used in case different images have the same crc
                                                image_name = str(crc)+"_"+str(id_num)+".png"
                                                while image_name in loaded_images: # check if image has already been loaded
                                                    if filecmp.cmp(image_name, image_path):
                                                        matched_image = True
                                                        break
                                                    id_num += 1
                                                    image_name = crc + "_" + id_num + ".png"
                                                if not matched_image:
                                                    shutil.copy(image_path, image_name)
                                            except FileNotFoundError:
                                                obj['render_voxon'] = False
                                                log.write("WARNING: "+obj.name+" not rendered, unable to find image texture at given path\n")
                                                some_not_rendered = True
                                                continue
                                            obj['image_name'] = image_name.encode('ascii')
                                            image_lookup[image_path] = obj['image_name']
                                            obj['voxon_material'] = 'IMAGE'
                                            obj['use_image'] = True
                                    else:
                                        obj['render_voxon'] = False
                                        log.write("WARNING: "+obj.name+" not rendered since no image texture was found\n")
                                        some_not_rendered = True
                                        continue
                                
                                elif color_node.bl_idname == 'ShaderNodeVertexColor':
                                    bl_cols_loc = mesh.color_attributes.active_color
                                    if bl_cols_loc is None:
                                        obj['render_voxon'] = False
                                        log.write("WARNING: "+obj.name+" not rendered since no color attribute was set\n")
                                        some_not_rendered = True
                                        continue
                                    obj['fillmode'] += 16
                                    obj['voxon_material'] = 'VERTEX'
                                    obj['use_cols'] = True
                                    
                                    if 'triangulated' not in obj or not obj['triangulated']:
                                        obj['triangulated'] = False
                                        for mod in obj.modifiers:
                                            if mod.type == 'TRIANGULATE':
                                                obj['triangulated'] = True
                                                break
                                        if not obj['triangulated']:
                                            obj.modifiers.new("Triangulate", "TRIANGULATE")
                                            obj['triangulated'] = True            
                                else:
                                    obj['render_voxon'] = False
                                    log.write("WARNING: "+obj.name+" not rendered due to unsupported shader node\n")
                                    some_not_rendered = True
                                    continue  
                            else:
                                obj['voxon_material'] = 'DEFAULT'
                        else:
                            obj['render_voxon'] = False
                            log.write("WARNING: "+obj.name+" not rendered due to unsupported shader node\n")
                            some_not_rendered = True
                            continue
                    else:
                        obj['render_voxon'] = False
                        log.write("WARNING: "+obj.name+" not rendered since not output shader node was found\n")
                        some_not_rendered = True
                        continue
                else:
                    obj['voxon_material'] = 'DIFFUSE'
            else:
                obj['voxon_material'] = 'NONE'
                obj['fillmode'] = 2
            
    except Exception:
        log.write("ERROR: Failed to parse materials, see traceback:\n" + traceback.format_exc())
        rec.close()
        log.close()
        os.chdir(cwd)
        return ({'ERROR'}, "Failed to parse materials, see log file")

    # Add audio if applicable
    audio_file = ""
    audio_fail = False
    try:
        if scene.sequence_editor:
            for seq in scene.sequence_editor.sequences_all:
                if seq.type == 'SOUND' and not seq.mute:
                    #add audio
                    audio_file = str(audio_num) + ".mp3"
                    bpy.ops.sound.mixdown(
                        filepath=audio_file,
                        check_existing=False,
                        relative_path=True, 
                        container='MP3',
                        codec='MP3',
                    )
                    log.write("INFO: Audio added under file "+audio_file+"\n")
                    break
    except Exception:
        log.write("WARNING: Failed to add audio, see traceback:\n" + traceback.format_exc())
        audio_file = ""
        audio_fail = True

    aspect = scene.voxon_properties.aspect
    show_viewbox = scene.voxon_properties.show_viewbox
    if show_viewbox:
        log.write("INFO: Viewbox boundaries being rendered in scene\n")
    
    frame_start = scene.frame_start
    frame_step = scene.frame_step
    frame_end = scene.frame_end
    frames_total = frame_end - frame_start + 1
    fps = scene.render.fps/scene.render.fps_base
    frame_time = frame_step/fps
    log.write("INFO: Viewbox aspect = "+str(tuple(aspect))+", effective frame rate: "+str(1/frame_time) + "\n")

    # Progress bar code
    window = bpy.context.window
    mouse_x = window.width
    mouse_y = window.height
    update_period = 5
    
    def progress_update(frames_rendered, percent, estimated_time):

        def draw(self, context):
            self.layout.label(text=str(frames_rendered)+"/"+str(frames_total)+" frames rendered ("+str(percent)+"%)")
            self.layout.label(text="Estimated time left: " + estimated_time)
            window.cursor_warp(mouse_x,mouse_y)

        bpy.context.window_manager.popup_menu(draw, title = "Voxon Recording", icon = 'INFO')
    
    def show_message(lines = [""], icon = 'INFO'):

        def draw(self, context):
            for line in lines:
                self.layout.label(text=line)

        bpy.context.window_manager.popup_menu(draw, title = "Voxon Recording", icon = icon)

    def render_frames():
        period_start = time.perf_counter()
        audio = audio_file
        while render_frames.frame <= frame_end:
            try:
                scene.frame_set(render_frames.frame)
                rec.write(render_frame(render_frames.anim_time, aspect, show_viewbox, old, audio))
                render_frames.anim_time += frame_time
                audio = "" # Only put in audio for first frame
            except Exception:
                log.write("ERROR: Failed to render frame "+str(render_frames.frame)+" , see traceback:\n" + traceback.format_exc())
                rec.close()
                log.close()
                os.chdir(cwd)
                show_message(["Failed to render frame "+str(render_frames.frame), "See log file for more details"], 'ERROR')
                props = bpy.context.scene.voxon_properties
                props.recording_output = "Last recording failed, see log file"
                return
            render_frames.frame += 1

            time_passed = time.perf_counter() - period_start
            if time_passed >= update_period:
                render_frames.time += time_passed
                frames_rendered = render_frames.frame - frame_start - 1
                percent = frames_rendered/frames_total
                estimated_time = round(render_frames.time/percent - render_frames.time)
                if estimated_time > 60:
                    estimated_time = str(estimated_time//60) + " min, "+str(estimated_time % 60) + " sec"
                else:
                    estimated_time = str(estimated_time) + " sec"
                percent = round(percent*100)
                progress_update(frames_rendered, percent, estimated_time)
                return 0 # Triggers update
        
        # Write pointer to end of file
        end_pos = rec.tell()
        rec.seek(8)
        rec.write(end_pos.to_bytes(4, 'little'))
        log.write(append_text + str(render_frames.anim_time) +"\n")
        make_time = time.perf_counter() - start_make_time
        log.write("INFO: Finished making recording! Took %.3f seconds" % round(make_time, 3))
        os.chdir(cwd)
        rec.close()
        log.close()
        end_message = ["Finished making recording!"]
        if some_not_rendered:
            end_message.append("Some objects not rendered, see log file for details")
        if audio_fail:
            end_message.append("Failed to add audio, see log file for details")
        show_message(end_message, 'ERROR')
        return

    render_frames.frame = frame_start
    render_frames.time = 0
    render_frames.anim_time = anim_time
    bpy.app.timers.register(render_frames)
    return({'INFO'}, 'Rendering frames')


def render_frame(time, aspect, show_viewbox, old, audio_file):
    dps = bpy.context.evaluated_depsgraph_get()
    
    msg = CmdFrameStart(time)
    msg += CmdSetView(aspect)
    if len(audio_file):
        msg += CmdPlaySound(audio_file)
    
    scene = bpy.context.scene
    viewbox = scene['VIEWBOX']
    matrix_viewbox = viewbox.matrix_world.inverted()
    if show_viewbox:
        msg += CmdDrawBox((-aspect[0],-aspect[1],-aspect[2],aspect[0],aspect[1],aspect[2]),1,0xffffff)

    # Get Objects
    for obj in scene.objects:
        if not obj['render_voxon'] or obj.hide_render:
            continue
        
        object_eval = obj.evaluated_get(dps) # get mesh with modifiers
        
        # Bounding Box Intersection Algorithm
        # 1. Transform object_eval into viewbox's world space using tester_mat
        # 2. Check if all corners are outside the closest x, y, and z planes of the viewbox
        # 3. If not, transform the viewbox into object_eval's world space
        # 4. Check if all corners of viewbox are outside the closest x, y, and z planes of the object
        # 5. If the answer to both checks is no, object is in frame
        in_frame = True
        for i in range(2):
            if i:   
                tester, testee = object_eval, viewbox
                tester_mat = tester.matrix_world.inverted() @ testee.matrix_world
            else: 
                tester, testee = viewbox, object_eval
                tester_mat = matrix_viewbox @ testee.matrix_world
            
            testee_corners = list(map(lambda c: tester_mat @ mathutils.Vector(c).to_4d(), testee.bound_box))
            # Determines which face to check against for each axis
            testee_pos = (testee_corners[0] + testee_corners[6]).to_3d() - \
                (mathutils.Vector(tester.bound_box[0]) + mathutils.Vector(tester.bound_box[6]))
            
            for axis in range(3):
                outside_face = True
                if testee_pos[axis] < 0:
                    bound = tester.bound_box[0][axis]
                    for c in testee_corners:
                        if c[axis] > bound:
                            outside_face = False
                            break
                elif testee_pos[axis] > 0:
                    bound = tester.bound_box[6][axis]
                    for c in testee_corners:
                        if c[axis] < bound:
                            outside_face = False
                            break
                else:
                    outside_face = False # At same position, no need to check
                if outside_face:
                    in_frame = False
                    break
            if not in_frame:
                break
        
        if not in_frame:
            continue

        mesh = object_eval.to_mesh()
        mesh.transform(obj.matrix_world)
        mesh.transform(matrix_viewbox)

        # Material properties
        color = 0xFFFFFF
        fillmode = obj['fillmode']
        uvc_bits = 0

        # make_recording should ensure this never fails
        # by properly setting material fields
        match obj['voxon_material']:
            case 'IMAGE':
                color = 0x404040

                if old:
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
            case 'DEFAULT':
                material = mesh.materials[0]
                output_node = material.node_tree.get_output_node('ALL')
                shader_node = output_node.inputs[0].links[0].from_node
                tri_color = shader_node.inputs[0].default_value
                color = int(tri_color[2] * 255)
                color += int(tri_color[1] * 255) << 8
                color += int(tri_color[0] * 255) << 16
            case 'DIFFUSE':
                tri_color = mesh.materials[0].diffuse_color
                color = int(tri_color[2] * 255)
                color += int(tri_color[1] * 255) << 8
                color += int(tri_color[0] * 255) << 16
            case 'VERTEX':
                bl_cols_loc = mesh.color_attributes.active_color
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
            case 'NONE':
                color = 0xFFFFFF

        if color == 0:
            break # Black will not render

        
        bl_verts = mesh.vertices
        verts = []
        for bl_vert in bl_verts:
            verts.append(-bl_vert.co[0])
            verts.append(bl_vert.co[1])
            verts.append(-bl_vert.co[2])
        
        obj['verts'] = verts
        vert_bits = (len(bl_verts) - 1).bit_length()
        
        index_bytes = (vert_bits + uvc_bits + 8)//8 # +7 makes it ceil, +1 for start_loop bit
        start_loop = 1 << (vert_bits + uvc_bits)

        meshes = b''
        polys = mesh.polygons
        
        if uvc_bits:
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
                
        msg += CmdDrawMeshTex(obj, fillmode, color, obj['image_name'])

    msg += CmdFrameEnd()
    return msg
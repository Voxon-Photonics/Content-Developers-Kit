from struct import pack
import numpy as np

endian = 'little'

# voxieplay commands:
def CmdFrameStart(time):
	cmd = b'F'
	data = pack('<f', time)
	return (cmd + data)

def CmdFrameEnd():
	cmd = b'E'
	return cmd
	
def CmdSetView(aspect):
	cmd = b'v'
	dims_np = np.array((-aspect[0],-aspect[1],-aspect[2],aspect[0],aspect[1],aspect[2]), dtype=np.float16)
	dims_bytes = dims_np.tobytes()
	return (cmd + dims_bytes)

def CmdDrawMeshTex(obj, fill =2 , base_color = 0xFFFFFFFF, image_name = b''):
	num_verts = len(obj['verts'])//3 # x // y = int(x/y)

	cmd = b'b'
	cmd += image_name + b'\x00'
	cmd += num_verts.to_bytes(4, endian)

	cmd += np.array(obj['verts'], dtype=np.float16).tobytes()

	if obj['use_image']:
		num_uvs = len(obj['uvs'])//2
		cmd += num_uvs.to_bytes(4, endian)
		cmd += np.array(obj['uvs'], dtype=np.float16).tobytes()
	else:
		cmd += b'\x00\x00\x00\x00'

	if obj['use_cols']:
		num_cols = len(obj['cols'])
		cmd += num_cols.to_bytes(4, endian)
		for col in obj['cols']:
			cmd += col.to_bytes(3, endian)
	else:
		cmd += b'\x00\x00\x00\x00'
	
	cmd += len(obj['meshes']).to_bytes(4, endian)
	cmd += obj['meshes']
	
	cmd += fill.to_bytes(1, endian)
	cmd += base_color.to_bytes(4, endian)
	return cmd

def CmdDrawBox(coords, fill, col):
	cmd = b'B'
	coords_bytes = np.array(coords, dtype=np.float16).tobytes()
	return cmd + coords_bytes + fill.to_bytes(1, endian) + col.to_bytes(4, endian)

def CmdInitialise():
	init = (b"KenSVXBX____") # underscores are where the file end pointer goes
	return (init)

def CmdRequestInfo():
	cmd = b'a'
	return cmd

def CmdSendFile(filepath, name):
	file = open(filepath, "rb")
	data = file.read()
	file.close()

	cmd = b'x' + name + b'\x00' + len(data).to_bytes(4, endian) + data
	return cmd

def CmdPlaySound(file_name, handle = 0, channel = -1, volume_left = 100, volume_right = 100, freq_mult = 1):
	cmd = b'e'

	cmd += file_name.encode('ascii') + b'\x00'
	cmd += handle.to_bytes(2, endian)
	cmd += channel.to_bytes(1, endian, signed = True)
	cmd += volume_left.to_bytes(2, endian, signed = True)
	cmd += volume_right.to_bytes(2, endian, signed = True)
	cmd += np.array([freq_mult], dtype=np.float16).tobytes()

	return cmd

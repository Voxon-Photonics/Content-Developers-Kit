# -*- coding: utf-8 -*-
"""
Created on Mon Jan 23 14:09:02 2023


Wrapper functions for Python -> VX Connection thru bigpak

This is a WIP 



Outgoing commands:

		//mandatory per frame:
	net_writef('F',"u4",framecnt); //voxie_frame_start
	net_writef('E',""); //voxie_frame_end

		//misc:
	net_writef('z',"fz",fnam); //voxie_mountzip
	net_writef('x',"fzu4{u1}",filnam,leng,buf); //special network command to send file image directly to voxieplay cache
	net_writef('l',"s2s2s2",r,g,b); //voxie_setleds
	net_writef('v',"f2f2f2f2f2f2",x0,y0,z0,x1,y1,z1); //voxie_setview
	net_writef('m',"f2f2f2f2f2f2",x0,y0,z0,nx,ny,nz); //voxie_setmaskplane
	net_writef('f',"fz",fnam); //voxie_free

		//render:
	net_writef('V',"f2f2f2u4",fx,fy,fz,col); //voxie_drawvox
	net_writef('B',"f2f2f2f2f2f2u1u4",x0,y0,z0,x1,y1,z1,fillmode,col); //voxie_drawbox
	net_writef('L',"f2f2f2f2f2f2u4",x0,y0,z0,x1,y1,z1,col); //voxie_drawlin
	net_writef('S',"f2f2f2f2u1u4",fx,fy,fz,rad,issol,col); //voxie_drawsph
	net_writef('c',"f2f2f2f2f2f2f2f2u1u4",x0,y0,z0,r0,x1,y1,z1,r1,issol,col); //voxie_drawcone
	net_writef('w',"fzh3h3h3h3u4f2f2u4",fnam,p,r,d,f,col,forcescale,fdrawratio,flags); //voxie_drawspr_ext
	net_writef('h',"fzh3h3h3h3u4u4u4",filnam,pp,rr,dd,ff,colorkey,reserved,flags); //voxie_drawheimap (NOTE: (flags&(1<<3) != 0) unsupported))
	net_writef('T',"h3h3h3u4sz",p,r,d,col,st); //voxie_printalph
	net_writef('C',"h3h3h3h3u1u4",p,r,d,f,fillmode,col); //voxie_drawcube
	net_writef('M',  "u2{h3u4}u2{u2}u1u4"       ,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex no texture (NOTE: (flags&(1<<3) != 0) unsupported))
	net_writef('N',"fzu2{h5u4}u2{u2}u1u4",texnam,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex w/ texture (NOTE: (flags&(1<<3) != 0) unsupported))
	net_writef('O',"fzu4{h5u4}u4{u4}u1u4",texnam,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex w/ texture (NOTE: (flags&(1<<3) != 0) unsupported))
	net_writef('o',"fzu4{F5u4}u4{u4}u1u4",texnam,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex w/ texture (NOTE: (flags&(1<<3) != 0) unsupported))

	  //audio:
	net_writef('A',"fzs1s2s2f2",filnam,chan,volperc0,volperc1,frqmul); //voxie_playsound
	net_writef('U',"s2s1s2s2f2",hand,chan,volperc0,volperc1,frqmul); //voxie_playsound_update

Incoming data:
	 //NOTE: Processing these bytes is optional, however it can be quite useful. For example: imagine a SpaceNav is connected to the VoxieBox.
	 //The remote app can receive the controls of the operator at the Voxiebox and adjust posori/zoom/etc without making the user walk back & forth.

		//04/09/2021: new controls format:
	char 'CtRl';
	(cmds: end w/char 0)
	{
			//Key & bstatus (char n fields always present even if 0):
		char 128-143; char keystat[n-128+1]; //scan codes, bstatus in 0xf1-0xf7
		char 144-159; char keybuf[n-160+1][3]; //3by:[0]:ASCII,[1]:scancode,[2]:Ct/Sh/Al

			//Mouse movement (only present when field is non-zero):
		char 1-7: bit mask for: short dmousx,dmousy,dmousz

			//Xbox controllers (only present when field is non-zero):
		char 15: char active_controllers; //bit mask 0..3
		char 16: short but  17:lt  18:rt  19:tx0  20:ty0  21:tx1  22:ty1  23:hat //#0
		char 24: short but  25:lt  26:rt  27:tx0  28:ty0  29:tx1  30:ty1  31:hat //#1
		char 32: short but  33:lt  34:rt  35:tx0  36:ty0  37:tx1  38:ty1  39:hat //#2
		char 40: short but  41:lt  42:rt  43:tx0  44:ty0  45:tx1  46:ty1  47:hat //#3

			//Spacenavs (only present when field is non-zero):
		char 48: short dx  49:dy  50:dz  51:ax  52:ay  53:az  54:char but //#0
		char 56: short dx  57:dy  58:dz  59:ax  60:ay  61:az  62:char but //#1
		char 64: short dx  65:dy  66:dz  67:ax  68:ay  69:az  70:char but //#2
		char 72: short dx  73:dy  74:dz  75:ax  76:ay  77:az  78:char but //#3
	}

@author: Matthew Vecchio
"""

from ctypes import *



## functions to help with VXConection



def float2half (f):
	bits = cast(pointer(c_float(f)), POINTER(c_int32)).contents.value
	sign = ((bits>>31)&1)
	expo = ((bits>>23)&255)-127
	base = (bits&((1<<23)-1))
	s = c_short()
	
	if (expo <=-15):
		s = (0<<10)
	elif (expo >= 16):
		s = (31<<10)
	else: #+/-inf
		s = ((expo+15)<<10) + (base>>13)
	
	return bytearray(c_short((sign<<15)+s))

def CmdFrameStart(time):
	cmd = 'F'.encode('ascii')
	data = bytearray(c_float(time))
	return (cmd + data)

def CmdFrameEnd():
	cmd = 'E'.encode('ascii')
	return cmd
	
def CmdSetView(x1,y1,z1, x2,y2,z2):
	cmd = 'v'.encode('ascii')
	return (cmd + 
				float2half(x1) + 
				float2half(y1) + 
				float2half(z1) + 
				float2half(x2) + 
				float2half(y2) + 
				float2half(z2)
				)

def CmdDrawMeshTex(obj, fill =2 , col = 0xFFFFFFFF):
	# 64 bit
	if('poltex' not in obj):
		return
		
	if(len(obj['poltex']) >= 1073741824 or len(obj['indices']) >= 1073741824):
		print("Mesh too large, consider splitting into submesh")
		return
	# 32 Bit
	elif(len(obj['poltex']) >= 32767 or len(obj['indices']) >= 32767):
		return CmdDrawMeshTex32Bit(obj, fill, col)
	# 16 Bit
	else:
		return CmdDrawMeshTex16Bit(obj, fill, col)
	
def CmdDrawMeshTex32Bit(obj, fill =2 , col = 0xFFFFFFFF):
	# timer_start = time.process_time()
	cmd = 'O'.encode('ascii')
	
	vertNo = len(obj['poltex'])
	cmd += bytearray(c_char(0))
	cmd += bytearray(c_uint32(vertNo)) # vertex count
	
	for vert in obj['poltex']:
		cmd += ( # Write 3 vertices
				float2half(vert[0]) + 
				float2half(vert[1]) + 
				float2half(vert[2]) + 
				float2half(vert[3]) + 
				float2half(vert[4]) + 
				bytearray(c_uint32(int(vert[5])))# Write 1 uint32
				)

	# confirm sizes / bit positions are correct
	# ind_timer = time.process_time()
	if(obj['recalc']):
		indNo = len(obj['indices'])
		cmd_indices = b''
		
		cmd_indices += bytearray(c_uint32(indNo)) # vertex count
			
		for index in obj['indices']:
			cmd_indices += bytearray(c_uint32(index))
			
		obj['cmd_indices'] = cmd_indices

	cmd += obj['cmd_indices']
	
	# Flags
	cmd += bytearray(c_char(fill))
	
	# Color
	cmd += bytearray(c_uint32(col))
	# M, Vert# (uint16), Verts [point3d, uint32], index# (uint16), indices[uint16], flags (unint8), col (uint32)
	
	#net_writef('M',"u2{h3u4}u2{u2}u1u4"		 ,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex no texture (NOTE: (flags&(1<<3) != 0) unsupported))
	#net_writef('M',"u2{h3u4}u2{u2}u1u4"			 ,i,&vt[0].x,24,&vt[0].col,24,j,mesh,4,2,0x404040); //voxie_drawmeshtex
	# print("\tBuild Mesh: {} ms".format((time.process_time() - timer_start) * 1000))
	return cmd

def CmdDrawMeshTex16Bit(obj, fill =2 , col = 0xFFFFFFFF):
	# timer_start = time.process_time()
	cmd = 'M'.encode('ascii')
	
	vertNo = len(obj['poltex'])
	# TODO V2 (Check Conversions)
	cmd += bytearray(c_uint16(vertNo)) # vertex count
	
	for vert in obj['poltex']:
		cmd += ( # Write 3 vertices
				float2half(vert[0]) + 
				float2half(vert[1]) + 
				float2half(vert[2]) + 
				bytearray(c_uint32(int(vert[5])))# Write 1 uint32
				)

	# confirm sizes / bit positions are correct
	# ind_timer = time.process_time()
	if(obj['recalc']):
		indNo = len(obj['indices'])
		cmd_indices = b''
	
		# TODO V2 (Check Conversions)
		cmd_indices += bytearray(c_uint16(indNo)) # vertex count
			
		for index in obj['indices']:
			cmd_indices += bytearray(c_uint16(index))
			
		obj['cmd_indices'] = cmd_indices

	cmd += obj['cmd_indices']
	
	# Flags
	cmd += bytearray(c_char(fill))
	
	# Color
	cmd += bytearray(c_uint32(col))
	# M, Vert# (uint16), Verts [point3d, uint32], index# (uint16), indices[uint16], flags (unint8), col (uint32)
	
	#net_writef('M',"u2{h3u4}u2{u2}u1u4"		 ,vtn,&vt[0].x,i,&vt[0].col,i,meshn,&mesh[0],sizeof(mesh[0]),flags,col); //voxie_drawmeshtex no texture (NOTE: (flags&(1<<3) != 0) unsupported))
	#net_writef('M',"u2{h3u4}u2{u2}u1u4"			 ,i,&vt[0].x,24,&vt[0].col,24,j,mesh,4,2,0x404040); //voxie_drawmeshtex
	# print("\tBuild Mesh: {} ms".format((time.process_time() - timer_start) * 1000))
	return cmd

def CmdDrawBox(x1,y1,z1, x2,y2,z2, fill, col):
	cmd = 'B'.encode('ascii')
	return (cmd + 
				float2half(x1) + 
				float2half(y1) + 
				float2half(z1) + 
				float2half(x2) + 
				float2half(y2) + 
				float2half(z2) + 
				bytearray(c_char(fill)) + 
				bytearray(c_uint32(col))
				)

def CmdInitialise():
	init = ("KenSVXBX:-P".encode('ascii') + bytes(1))
	return (init)
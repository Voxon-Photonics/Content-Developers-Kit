# -*- coding: utf-8 -*-
"""
Created on Wed Oct 28 15:30:05 2020

@author: Voxon-Ben
"""
from Suzanne import Suzanne, POLTEX_T

from ctypes import Structure, c_float, c_int, c_double, c_void_p, c_char_p, WinDLL, byref, POINTER

MAXDISP =  3 

class POINT2D(Structure):
	_fields_ = [("x", c_float),
			 ("y", c_float)]

class POINT3D(Structure):
	_fields_ = [("x", c_float),
			 ("y", c_float),
			 ("z", c_float),]

class VOXIE_DISP_T(Structure):
	_fields_ = [
			("keyst", POINT2D * 8),
			("colo_r", c_int),
			("colo_g", c_int),
			("colo_b", c_int),
			("mono_r", c_int),
			("mono_g", c_int),
			("mono_b", c_int),
			("mirrorx", c_int),
			("mirrory", c_int)
			]
			  
class VOXIE_WIND_T(Structure):
	_fields_ = [
			("useemu", c_int),
			("emuhang", c_float),
			("emuvang", c_float),
			("emudist", c_float),
			("xdim", c_int),
			("ydim", c_int),
			("projrate", c_int),
			("framepervol", c_int),
			("usecol", c_int),
			("dispnum", c_int),
			("bitspervol", c_int),
			("disp", VOXIE_DISP_T * MAXDISP),
			("hwsync_frame0", c_int),
			("hwsync_phase", c_int),
			("hwsync_amp", c_int * 4),
			("hwsync_pha", c_int * 4),
			("hwsync_levthresh", c_int),
			("voxie_vol", c_int),
			("ilacemode", c_int),
			("drawstroke", c_int),
			("dither", c_int),
			("smear", c_int),
			("usekeystone", c_int),
			("flip", c_int),
			("menu_on_voxie", c_int),
			("aspx", c_float),
			("aspy", c_float),
			("aspz", c_float),
			("gamma", c_float),
			("density", c_float),			 
			("sndfx_vol", c_int),
			("voxie_aud", c_int),
			("excl_audio", c_int),
			("sndfx_aud", c_int*2),
			("playsamprate", c_int),
			("playnchans", c_int),
			("recsamprate", c_int),
			("recnchans", c_int),			 
			("isrecording", c_int),
			("hacks", c_int),
			("dispcur", c_int),
			("freq", c_double),
			("phase", c_double),
			("thread_override_hack", c_int),
			("motortyp", c_int),
			("clipshape", c_int),
			("goalrpm", c_int),
			("cpmaxrpm", c_int),
			("ianghak", c_int),
			("ldotnum", c_int),
			("reserved0", c_int),
			("upndow", c_int),
			("nblades", c_int),
			("usejoy", c_int),
			("dimcaps", c_int),
			("emugam", c_float),
			("asprmin", c_float),
			("sync_usb_offset", c_float),
			("sensemask", c_int * 3),
			("outcol", c_int * 3),
			("aspr", c_float),
			("sawtoothrat", c_float)]

class TILETYPE_T(Structure):
	_fields_ = [
			("f", c_void_p),
			("p", c_void_p),
			("x", c_void_p),
			("y", c_void_p)
			]
	
class VOXIE_FRAME_T(Structure):
	_fields_ = [
			("f", c_void_p),
			("p", c_void_p),
			("fp", c_void_p),
			("x", c_int),
			("y", c_int),
			("usecol", c_int),
			("drawplanes", c_int),
			("x0", c_int),
			("y0", c_int),
			("x1", c_int),
			("y1", c_int),
			("xmul", c_float),
			("ymul", c_float),
			("zmul", c_float),
			("xadd", c_float),
			("yadd", c_float),
			("zadd", c_float),
			("f2d", TILETYPE_T)
			]

class VOXIE_INPUTS_T(Structure):
	_fields_ = [
			("bstat", c_int),
			("obstat", c_int),
			("dmousx", c_int),
			("dmousy", c_int),
			("dmousz", c_int),
			]

suzanne = Suzanne()

class Runtime:
	def __init__(self):
		self.vxDLL = WinDLL("C:/Voxon/System/Runtime/voxiebox.dll")
		self.vw = VOXIE_WIND_T()
		self.in_t = VOXIE_INPUTS_T()
		self.vf = VOXIE_FRAME_T()
		
		self.vxDLL.voxie_loadini_int(byref(self.vw))
		self.vxDLL.voxie_init(byref(self.vw))
		
		self.vxDLL.voxie_setview.argtypes = [POINTER(VOXIE_FRAME_T), c_float, c_float, c_float, c_float, c_float, c_float]
		self.vxDLL.voxie_drawbox.argtypes = [POINTER(VOXIE_FRAME_T), c_float, c_float, c_float, c_float, c_float, c_float, c_int, c_int]
		self.vxDLL.voxie_drawmeshtex.argtypes = [POINTER(VOXIE_FRAME_T), c_char_p, POINTER(POLTEX_T), c_int, POINTER(c_int), c_int, c_int, c_int]

	# Does work as a loop; better to have consumer as loop?
	def Loop(self):
		while (self.vxDLL.voxie_breath(byref(self.in_t)) == 0):
			if self.vxDLL.voxie_keystat(0x1):
				self.vxDLL.voxie_quitloop();
			
			self.vxDLL.voxie_frame_start(byref(self.vf));
			aspx_min = -self.vw.aspx
			aspy_min = -self.vw.aspy
			aspz_min = -self.vw.aspz
			aspx_max = self.vw.aspx
			aspy_max = self.vw.aspy
			aspz_max = self.vw.aspz
			
			self.vxDLL.voxie_setview(byref(self.vf), aspx_min, aspy_min, aspz_min, aspx_max, aspy_max, aspz_max);
			
			self.vxDLL.voxie_drawbox(
					byref(self.vf), 
					aspx_min + 0.001, 
					aspy_min + 0.001, 
					aspz_min, 
					aspx_max - 0.001, 
					aspy_max - 0.001, 
					aspz_max, 
					1, 0xffffff);
					
			self.vxDLL.voxie_drawmeshtex(byref(self.vf), None, suzanne.Pol, suzanne.PCount, suzanne.IDX, suzanne.ICount, 2, 0xffffff)
			self.vxDLL.voxie_frame_end();
			self.vxDLL.voxie_getvw(byref(self.vw));

		self.vxDLL.voxie_uninit_int(0);
		self.running = False
		
runtime = Runtime()
runtime.Loop()
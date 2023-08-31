# -*- coding: utf-8 -*-
"""
Created on Wed Nov 30 14:15:05 2022

@author: Mathew Vecchio for Voxon
"""

# various data types for Voxon Development using Python.
# Note all class names are in lowercase and written in the same style as voxiebox.h to make cut and paste of functions easier. 
#
# Types are based on the June 27th 2023 Runtime

from ctypes import * 

MAXDISP =  3 


class point2d(Structure):
	_fields_ = [("x", c_float),
			 ("y", c_float)]

class point3d(Structure):
	_fields_ = [("x", c_float),
			 ("y", c_float),
			 ("z", c_float),]

class voxie_disp_t(Structure):
	_fields_ = [
			("keyst", point2d * 8),
			("colo_r", c_int),
			("colo_g", c_int),
			("colo_b", c_int),
			("mono_r", c_int),
			("mono_g", c_int),
			("mono_b", c_int),
			("mirrorx", c_int),
			("mirrory", c_int)
			]
			  
class voxie_wind_t(Structure):
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
			("disp", voxie_disp_t * MAXDISP),
			("hwsync_frame0", c_int),
			("hwsync_phase", c_int),
			("hwsync_amp", c_int * 4), ## this is how you define an array in ctypes
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
			("normhax", c_int),
			("upndow", c_ubyte ),
            ("scrshape", c_ubyte ),
            ("filler", c_ubyte * 2 ),
			("nblades", c_int ),
			("usejoy", c_int),
			("dimcaps", c_int),
			("emugam", c_float),
			("asprmin", c_float),
			("sync_usb_offset", c_float),
			("sensemask", c_int * 3),
			("outcol", c_int * 3),
			("aspr", c_float),
			("sawtoothrat", c_float)]

class tiletype_t(Structure):
	_fields_ = [
			("f", c_void_p),
 			("p", c_void_p),
			("x", c_void_p),
			("y", c_void_p)
			]
	
class voxie_frame_t(Structure):
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
			("f2d", tiletype_t)
			]

class voxie_input_t(Structure):
	_fields_ = [
			("bstat", c_int),
			("obstat", c_int),
			("dmousx", c_int),
			("dmousy", c_int),
			("dmousz", c_int),
			]
    
class voxie_xbox_t(Structure):
    _fields_ = [
            ("but", c_short),
            ("lt", c_short),
            ("rt", c_short),
            ("tx0", c_short),
            ("ty0", c_short),
            ("tx1", c_short),
            ("ty1", c_short),
            ("hat", c_short),       
            ]

class voxie_nav_t(Structure):
    _fields_ = [
            ("dx", c_float),
            ("dy", c_float),
            ("dz", c_float),
            ("ax", c_float),
            ("ay", c_float),
            ("az", c_float),
            ("but", c_int),    
            ]


class touchkey_t(Structure):
    _fields_ = [
            ("st", POINTER(c_char_p) ),
            ("x0", c_int),
            ("y0", c_int),
            ("xs", c_int),
            ("ys", c_int),
            ("fcol", c_int),
            ("bcol", c_int),
            ("keycode", c_int),
            ]

class pol_t(Structure):
    _fields_ = [
        ("x", c_float),
        ("y", c_float),
        ("z", c_float),
        ("p2", c_int),
        ]
    
class poltex_t(Structure):
    _fields_ = [
        ("x", c_float),
        ("y", c_float),
        ("z", c_float),
        ("u", c_float),
        ("v", c_float),
        ("col", c_int)
        ]

class extents_t(Structure):
    _fields_ = [
        ("x0", c_float),
        ("y0", c_float),
        ("z0", c_float),
        ("sc", c_float),
        ("x1", c_float),
        ("y1", c_float),
        ("z1", c_float),
        ("dum", c_float)
        ]

VOXIE_DICOM_MIPS =  4 #NOTE:limited to 6 by voxie_dicom_t:gotz bit mask trick

   
class kzfile_t(Structure):
    _fields_ = []
        


class voxie_dicom_t(Structure):
    _fields_ = [ 
        ("mip", POINTER(c_ushort * VOXIE_DICOM_MIPS)), ## maybe this pointer wont work 
        ("gotz", POINTER(c_int)),
        ("rulerPos", point3d * 3), 
        ("slicep", point3d),
        ("slicer", point3d),
        ("sliced", point3d),
        ("slicef", point3d),
        ("reserved", c_float),      
        ("timsincelastmove", c_float),      
        ("detail", c_float),      
        ("level", c_float * 2),      
        ("xsiz", c_int),      
        ("ysiz", c_int),              
        ("zsiz", c_int),              
        ("zloaded", c_int),              
        ("color", c_int * 2),    
        ("zloaded", c_int),     
        ("autodetail", c_int),     
        ("usedmip", c_int), 
        ("slicemode", c_int), 
        ("drawstats", c_int), 
        ("ruler", c_int), 
        ("flags", c_int),         
        ("defer_load_posori", c_int),       
        ("gotzip", c_int),       
        ("validfil", c_int),       
        ("forcemip", c_int),    
        ("lockmip", c_int),  
        ("saveasstl", c_int),  
        ("nfiles", c_int),
        ("n", c_int),
        ("cnt", c_int),
        ("nummin", c_int),
        ("nummax", c_int),
        ("filespec", c_char * 260),
        ("xsc", c_float),
        ("ysc", c_float),
        ("zsc", c_float),
        ("kzfil", POINTER(kzfile_t)),
        ("valsumn", c_int64),
        ("valsums", c_int64),
        ("valsumss", c_int64),
        ("valmin", c_int),
        ("valmax", c_int),   
        ("compmode", c_int),
        ("ncomp", c_int),  
        ("multiframe", c_int),
        ("mirrorz", c_int),  
        ("zslice", c_int),
        ("zslice0", c_int), 
        ("zslicemin", c_int),
        ("zslicemax", c_int),  
        ("bitmal", c_int),
        ("bitmsb", c_int),  
        ("bituse", c_int),
        ("issign", c_int),  
        ("reslop", c_float),  
        ("reinte", c_float),  
        ("wincen", c_float),  
        ("winwid", c_float), 
        ("usepal", c_int), 
        ("winwid", c_int * 256)
        ]
         


class voxie_rec_t(Structure):
    _fields_ = [
        ("kzfil", POINTER(kzfile_t)),
        ("timleft", c_double),
        ("frametim", POINTER(c_float)),
        ("playspeed", c_float),
        ("frameseek", POINTER(c_int)),
        ("framemal", c_int),
        ("kztableoffs", c_int),
        ("error", c_int),
        ("playmode", c_int),
        ("framecur", c_int),
        ("framenum", c_int),
        ("currep", c_int),
        ("animode", c_int),
        ("playercontrol", c_int * 4)         
        ]

class menu_update_t(Structure):
	_fields_ = [
		("id",  c_int),
		("st", c_char_p),
		("val", c_double),
	]


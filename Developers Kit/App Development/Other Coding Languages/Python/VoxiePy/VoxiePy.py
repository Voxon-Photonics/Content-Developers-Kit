# -*- coding: utf-8 -*-
"""

VoxiePy - Python wrapper for Voxon Development. 

see VoxiePy Example folder to learn how to use / to make your own apps. 

VxPy is the same function callina as standard VX development *see voxiebox.txt for a list of function calls / descriptions
located at Developers Kit \ App Development \ C Voxon SDK Development \ Documentation \ voxiebox.txt

For extra helper functions see VxPyTools module and examples.

Be sure to add these modules to your working directories for easy access.

If you develop something cool with VxPy be sure to show me at Matt@voxon.co 

@author: Matthew Vecchio / ReadyWolf
"""

from VxPyDataTypes import  *

from ctypes import * 

import math
import random

DEBUG = False
TEST = False

"""

create - BREATH LOOP - Done
create - START FRAME - Done
create - END FRAME - Done
create - SHUTDOWN - Done
Create DEBUG_TEXT function - Done
Add Show Stats Function - Done
Add all standard VX calls - Done



TODO

WishList :

* Work out a way to do function pointers for playback audio and menu functions 










"""


class VoxiePy:    
      
    def __init__(self):
        
               try:
                   self.vxDLL = WinDLL("C:/Voxon/System/Runtime/voxiebox.dll")
                   self.k32 = WinDLL('kernel32') ## load in the Windows Kernel so we can ensure the voxiebox.dll is freed
                   self.DLLloaded = True
                   print("                    ____")
                   print("                 .'`_ o `;__,")
                   print("       .       .'.'` '---'  ' ")
                   print("       .`-...-'.'        VoxiePy - Voxon Python Wrapper  ")
                   print("        `-...-'   for VX Runtime June 27th 2023")
                   print(" ")    
                   
                   print("VoxiePy - Voxon Python Wrapper ::")
                   print("VoxieBox.DLL found and loaded into memory...")
               except Exception as e:
                   print("Error can't find / access voxiebox.dll")
                   print(e)
                   print("\n\n\n")
                   self.DLLloaded = False
                   return 
               
                
               try:
                   self.ScanCodeInit()
                   
                   ### DEFINE ALL THE VOXIE FUNCTIONS ARGTYPES & RETURN TYPES
                           
                # Core
                    
                   # initalises the VX instance. args = (pointer to voxie_wind_t)
                   self.vxDLL.voxie_init.argtypes = [POINTER(voxie_wind_t)]
                   # loads in the settings from voxiebox.ini and voxiebox_menu0.ini args = (pointer to voxie_wind_t the VX instance. args = (pointer to voxie_wind_t)      
                   self.vxDLL.voxie_loadini_int.argtypes = [POINTER(voxie_wind_t)]
                   self.vxDLL.voxie_getvw.int_argtypes = [POINTER(voxie_wind_t)]
                   self.vxDLL.voxie_uninit_int.argtypes = [c_int]
                   self.vxDLL.voxie_mountzip.argtypes = [c_char_p]
                   self.vxDLL.voxie_free.argtypes = [c_char_p]
                   self.vxDLL.voxie_getversion.retypes = [c_int64]
                   self.vxDLL.voxie_setview.argtypes = [POINTER(voxie_frame_t),  c_float, c_float, c_float, c_float, c_float, c_float]
                   self.vxDLL.voxie_breath.argtypes = [POINTER(voxie_input_t)]
                   self.vxDLL.voxie_breath.retypes = c_int
                   self.vxDLL.voxie_klock.retypes = c_double
    
    
                # Input
                
                   self.vxDLL.voxie_keystat.argtypes = [c_int]
                   self.vxDLL.voxie_keystat.retypes = c_int
                   self.vxDLL.voxie_keyread.retypes = c_int
                   self.vxDLL.voxie_xbox_read.argtypes = [c_int, POINTER(voxie_xbox_t)]
                   self.vxDLL.voxie_xbox_read.retypes = c_int
                   self.vxDLL.voxie_xbox_write.argtypes = [c_int, c_float, c_float]
                   self.vxDLL.voxie_nav_read.argtypes = [c_int, POINTER(voxie_nav_t)]
                   self.vxDLL.voxie_nav_read.retypes = c_int
                   self.vxDLL.voxie_touch_read.argtypes = [POINTER(c_int), POINTER(c_int), POINTER(c_int), POINTER(c_int)]
                   self.vxDLL.voxie_touch_read.retypes = c_int
                   self.vxDLL.voxie_touch_custom.argtypes = [c_char_p, c_int, c_int, c_int, c_int, c_int, c_int, c_int]

    
           
              #  voxie_drawspr = (int(__cdecl*)(voxie_frame_t*, const char*, point3d*, point3d*, point3d*, point3d*, int)
                    
                # Graphics 3D
                
                   #Low
                  
                   self.vxDLL.voxie_setleds.argtypes = [c_int,  c_int, c_int, c_int] 
                   self.vxDLL.voxie_setnorm.argtypes = [c_float, c_float, c_float]
                   
                   #Prims
                   
                   self.vxDLL.voxie_drawvox.argtypes = [POINTER(voxie_frame_t), c_float, c_float, c_float, c_int]
                   self.vxDLL.voxie_drawbox.argtypes = [POINTER(voxie_frame_t), c_float, c_float, c_float, c_float, c_float, c_float, c_int, c_int]
                   self.vxDLL.voxie_drawlin.argtypes = [POINTER(voxie_frame_t), c_float, c_float, c_float, c_float, c_float, c_float, c_int]
                   # draw cube voxie_drawcube @params = (vf, pos*, rVec*, dVec*, fVec*, fill, col)
                   self.vxDLL.voxie_drawcube.argtypes = [POINTER(voxie_frame_t), POINTER(point3d), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_int, c_int]
                   # draw sphere @params = (vf, posx, posy, posz, rad, fill, col)
                   self.vxDLL.voxie_drawsph.argtypes = [POINTER(voxie_frame_t), c_float, c_float, c_float, c_float, c_int, c_int]
                   self.vxDLL.voxie_drawcone.argtypes = [POINTER(voxie_frame_t), c_float, c_float, c_float, c_float, c_float, c_float, c_float, c_float, c_int, c_int]
                   self.vxDLL.voxie_drawpol.argtypes = [POINTER(voxie_frame_t), POINTER(pol_t), c_int, c_int]
    
                   #Text
                   self.vxDLL.voxie_printalph.argtypes = [POINTER(voxie_frame_t), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_int, c_char_p] 
                   self.vxDLL.voxie_printalph_ext.argtypes = [POINTER(voxie_frame_t), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_float, c_int, c_char_p]               
       
                   #Mesh
                   self.vxDLL.voxie_drawmeshtex.argtypes = [POINTER(voxie_frame_t),  c_char_p, POINTER(poltex_t), c_int, POINTER(c_int), c_int, c_int, c_int] 
                   self.vxDLL.voxie_drawspr.argtypes = [POINTER(voxie_frame_t), c_char_p, POINTER(point3d), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_int]
                   self.vxDLL.voxie_drawspr.retypes = c_int
                   self.vxDLL.voxie_drawspr_ext.argtypes = [POINTER(voxie_frame_t), c_char_p, POINTER(point3d), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_int, c_float, c_float, c_int]
                   self.vxDLL.voxie_drawspr_ext.retypes = c_int
                   self.vxDLL.voxie_drawheimap.argtypes = [POINTER(voxie_frame_t), c_char_p, POINTER(point3d), POINTER(point3d), POINTER(point3d), POINTER(point3d), c_int, c_int, c_int]
                   self.vxDLL.voxie_drawheimap.retypes = c_float
                   self.vxDLL.voxie_drawspr_getextents.argtypes =[c_char_p, POINTER(extents_t), c_int]
                   self.vxDLL.voxie_drawspr_getextents.retypes = c_int

                   #DICOM
                   self.vxDLL.voxie_drawdicom.argtypes =[POINTER(voxie_frame_t),POINTER(voxie_dicom_t), c_char_p, POINTER(point3d),POINTER(point3d),POINTER(point3d),POINTER(point3d),POINTER(c_int),POINTER(c_int)]
               
                   
                # Graphics 2D
                   self.vxDLL.voxie_debug_print6x8.argtypes = [c_int, c_int, c_int, c_int, c_char_p]
                   self.vxDLL.voxie_debug_drawpix.argtypes = [c_int, c_int, c_int]
                   self.vxDLL.voxie_debug_drawhlin.argtypes = [c_int, c_int, c_int, c_int]
                   self.vxDLL.voxie_debug_drawline.argtypes = [c_int, c_int, c_int, c_int, c_int]
                   self.vxDLL.voxie_debug_drawcirc.argtypes = [c_int, c_int, c_int, c_int]  
                   self.vxDLL.voxie_debug_drawrectfill.argtypes = [c_int, c_int, c_int, c_int, c_int]     
                   self.vxDLL.voxie_debug_drawcircfill.argtypes = [c_int, c_int, c_int, c_int]   
                   self.vxDLL.voxie_debug_drawtile.argtypes = [POINTER(tiletype_t), c_int, c_int]   
                   
                # Sound   
                   self.vxDLL.voxie_playsound.argtypes = [c_char_p, c_int, c_int, c_int, c_float]
                   self.vxDLL.voxie_playsound.retypes = c_int
                   self.vxDLL.voxie_playsound_update.argtypes = [c_int, c_int, c_int, c_int, c_float]
                   self.vxDLL.voxie_playsound_seek.argtypes = [c_int, c_double, c_int]
     
                # Rec
                   self.vxDLL.voxie_rec_open.argtypes = [POINTER(voxie_rec_t), POINTER(voxie_wind_t), c_char_p, c_char_p, c_int ]
                   self.vxDLL.voxie_rec_open.retypes = c_int
                   self.vxDLL.voxie_rec_play.argtypes = [POINTER(voxie_rec_t), POINTER(voxie_wind_t), c_int ]
                   self.vxDLL.voxie_rec_play.retypes = c_int                
                   self.vxDLL.voxie_rec_close.argtypes = [POINTER(voxie_rec_t)]  
                
                # Menu Functions  & Sound functions that aren't working yet :( ) -- currently no supported. Again -- how do you pass in user functions as function ptrs?
                # self.vxDLL.voxie_setaudplaycb.argtypes = [POINTER(userplayfunc),POINTER(c_int), c_int] ## how do you pass in user functions as function ptrs?
                # self.vxDLL.voxie_setaudreccb.argtypes = [POINTER(userrecfunc),POINTER(c_int), c_int]   ##how do you pass in user functions as function ptrs?

                # voxie_menu_reset @params (int (*menu_update)(int id, char *st, double val, int how, void *userdata), void *userdata, char *bkfilnam);
                ##   self.vxDLL.voxie_menu_reset.argtypes = [POINTER(menu_update_t), c_void_p, c_void_p ]
                # voxie_menu_addtab @params (const char *st, int x, int y, int xs, int ys);
                ##   self.vxDLL.voxie_menu_addtab.argtypes = [c_char_p, c_int, c_int, c_int, c_int] 
                # voxie_menu_additem @params (const char *st, int x, int y, int xs, int ys, int id, int type, int down, int col, double v, double v0, double v1, double vstp0, double vstp1);
                ##   self.vxDLL.voxie_menu_additems.argtypes = [c_char_p, c_int, c_int, c_int, c_int,c_int, c_int, c_int, c_int,c_double,c_double,c_double,c_double,c_double] 
                # voxie_menu_updateitem @params (int id, char *st, int down, double v);
                ##   self.vxDLL.voxie_menu_updateitem.argtypes = [c_int, c_char_p, c_int, c_double];

               
                   ### self up core runtime variables 
               
                   self.vw = voxie_wind_t()
                   self.in_t = voxie_input_t()
                   self.vf = voxie_frame_t()
                   
                   self.time = self.vxDLL.voxie_klock()
                   self.oldTime =  self.vxDLL.voxie_klock()
                   self.deltaTime =  0
                   self.averageTime = 0
                    
                   
                   ### load in ini settings
                    
                   self.vxDLL.voxie_loadini_int(byref(self.vw))
                   
                   ### Initalise voxiewindow
                   
                   self.vxDLL.voxie_init(byref(self.vw))
               		
                   
                   print("VoxiePy - initialized successfully")
               except Exception as e:
                    print("VoxiePy - initialized unsuccessfully")
                    print(e)
                  
                    self.Uninit()
                    return 
               
    def Uninit(self):       
        self.vxDLL.voxie_quitloop()  
        success = True
        if self.DLLloaded == True: 
            try:
                self.vxDLL.voxie_uninit_int(0)
                print("voxie_uninit_int called successfuly")
            except Exception as e:
                print("Error : couldn't call voxie_uninit_int()")
                print(e)
                success = False
            try:
                self.k32.FreeLibrary.argtypes = [c_void_p]
                self.k32.FreeLibrary(self.vxDLL._handle) 
                print("VoxieBox.DLL freed")
            except Exception as e:
                print("Error  : couldn't call k32 Free Library()!")
                print(e)               
                success = False
                 
        if success:
            print("VoxiePy quit successfully");      
        else:
            print("Error : VoxiePy did not exit cleanly...")
        self.__del__()

    def __del__(self):
        pass

    
    def CoreUpdate(self):
        try:
            self.oldTime = self.time
            self.time = self.vxDLL.voxie_klock() *.0000001
            self.deltaTime = self.time - self.oldTime
            self.averageTime += (self.deltaTime - self.averageTime) *  .1
            
            if self.vxDLL.voxie_keystat(0x1):
                self.vxDLL.voxie_quitloop()  
                          
                            
            ## emulator controls
            #i = (self.vxDLL.voxie_keystat(0x1b) == 1) - (this->voxie_keystat(0x1a) # keys '[' and ']'
            i = 0
            if self.vxDLL.voxie_keystat(0x1b) : i = 1
            if self.vxDLL.voxie_keystat(0x1a) : i = -1
            
            
            if self.vxDLL.voxie_keystat(0x2a) > 0 or self.vxDLL.voxie_keystat(0x36) > 0 :
                self.vw.emuvang = min(max(self.vw.emuvang + i * self.deltaTime * 2.0, -3.14159265358979323 * .5), 0.1268) # //Shift+[,]      
            
            elif self.vxDLL.voxie_keystat(0x1d) > 0 or self.vxDLL.voxie_keystat((0x9d)) > 0 :
                self.vw.emudist = max(self.vw.emudist - i * self.deltaTime * 2048.0, 400.0) # //Ctrl+[,]
            else:        
                self.vw.emuhang += i * self.deltaTime * 2
            
            if i != 0 :            
                self.vxDLL.voxie_init(byref(self.vw))

        except Exception as e:
            print("Error  : Couldn't perform VoxiePy core update")
            print(e)         
            self.Uninit()
    
    
    def Breath(self):
      
        self.CoreUpdate()
        return self.vxDLL.voxie_breath(byref(self.in_t))
    
    
    def StartFrame(self):
        self.vxDLL.voxie_frame_start(byref(self.vf))
        self.vxDLL.voxie_setview(byref(self.vf), -self.vw.aspx, -self.vw.aspy,  -self.vw.aspz,  self.vw.aspx,  self.vw.aspy,  self.vw.aspz)


    def EndFrame(self):
        self.vxDLL.voxie_frame_end(byref(self.vf))
        self.vxDLL.voxie_getvw(byref(self.vw))
        
        
    def ShowStats(self, posx, posy):
        

        try:
            self.vxDLL.voxie_debug_print6x8(posx, posy, 0x00ff00,0x003300, c_char_p(b"VoxiePy --====e"))
            posy += 8
            msg = "Runtime : {:.3} VPS : {:.3}".format(float(self.time), float(1/self.averageTime))
            msg_b = bytes(msg,'UTF-8')
            
            self.vxDLL.voxie_debug_print6x8(posx, posy, 0x0ff80,0x003300, c_char_p(msg_b))
            posy += 8
            msg = "Delta : {:.3}".format(float(self.deltaTime))
            msg_b = bytes(msg,'UTF-8')
            self.vxDLL.voxie_debug_print6x8(posx, posy, 0x00ff80,0x003300, c_char_p(msg_b))
        except Exception as e:
            print("Error in VoxiePy's ShowStats() function")
            print(e)
            
            
    # A Function used to test if the wrapper is working
    def Test(self):
        

        print("VXC Test begin")
        
        if self.DLLloaded == False: return

        print("DLL loaded", self.vxDLL.voxie_breath(byref(self.in_t)))

 
        
        while (self.vxDLL.voxie_breath(byref(self.in_t)) == 0):
         
            
            self.CoreUpdate()
            
            print("VX Breathing... Running for {:^.4}".format(float(self.time)))

  
            ## start frame - all draw calls after this line
            self.vxDLL.voxie_frame_start(byref(self.vf))
                
            self.vxDLL.voxie_setview(byref(self.vf), -self.vw.aspx, -self.vw.aspy, -0.4, 1, 1, 0.4)
              
            ## draw box around  the outside of the box
            self.vxDLL.voxie_drawbox(byref(self.vf), 
    					-self.vw.aspx + 0.001, 
    					-self.vw.aspy + 0.001, 
    					-self.vw.aspz, 
    					self.vw.aspx - 0.001, 
    					self.vw.aspy - 0.001, 
    					self.vw.aspz, 
    					1, 0xffffff)
            
            
            self.ShowStats(0, 500)
    					
    
            ## end frame - No more VX draw calls after this  line
            self.vxDLL.voxie_frame_end()
                
            self.vxDLL.voxie_getvw(byref(self.vw))

    
    ## this functions initalises all the scan codes for various input types and enums used in the system
    def ScanCodeInit(self): 
        
        
        ## Joy / Gamepad the bit value button codes to work with Xbox / Game controllers  
        
        self.JOY_DPAD_UP			= c_int(0) 	# bit 0 dec value 1			Digital Dpad Up				
        self.JOY_DPAD_DOWN			= c_int(1) 	# bit 1 dec value 2			Digital Dpad Down 
        self.JOY_DPAD_LEFT			= c_int(2) 	# bit 2 dec value 4			Digital Dpad Left
        self.JOY_DPAD_RIGHT			= c_int(3) 	# bit 3 dec value 8			Digital Dpad Right
        self.JOY_START				= c_int(4) 	# bit 4 dec value 16		Start Button
        self.JOY_BACK				= c_int(5) 	# bit 5 dec value 32		Back Button
        self.JOY_LEFT_THUMB			= c_int(6) 	# bit 6 dec value 64		Left Thumb Stick Button (when you press 'down' on the left analog stick)
        self.JOY_RIGHT_THUMB		= c_int(7) 	# bit 7 dec value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
        self.JOY_LEFT_SHOULDER		= c_int(8)	# bit 8 dec value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
        self.JOY_RIGHT_SHOULDER		= c_int(9)	# bit 9 dec value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
        self.JOY_BUT_A				= c_int(12)	# bit 12 dec value 1,024	The 'A' Button on a standard Xbox Controller
        self.JOY_BUT_B				= c_int(13)	# bit 13 dec value 2,048	The 'B' Button on a standard Xbox Controller
        self.JOY_BUT_X				= c_int(14)	# bit 14 dec value 4,096	The 'X' Button on a standard Xbox Controller
        self.JOY_BUT_Y				= c_int(15)	# bit  15 dec value 8,192	The 'Y' Button on a standard Xbox Controller

        # enum Joy Controller Axis Enum helps identify which analog input is being used 
        
        self.JOY_STICK_LEFT_X = c_int(0)			# Analog Left Stick X axis
        self.JOY_STICK_LEFT_Y = c_int(1)			# Analog Left Stick Y axis
        self.JOY_STICK_RIGHT_X = c_int(2)			# Analog Right Stick X axis
        self.JOY_STICK_RIGHT_Y = c_int(3)			# Analog Right Stick Y axis
        
        self.JOY_TRIG_LEFT = c_int(0)	# 0 Analog Left Shoulder trigger 
        self.JOY_TRIG_RIGHT = c_int(1)	# 1 Analog Right Shoulder trigger

        ## Mouse

        self.MOUSE_BUT_LEFT = c_int(0)		# bit 0 value 1			Left Mouse Button
        self.MOUSE_BUT_RIGHT = c_int(1)		# bit 1 value 2			Right Mouse Button
        self.MOUSE_BUT_MIDDLE = c_int(2)	# bit 2 value 4			Middle Mouse Button
        
        
        ## Space Nav
        
        self.NAV_AXIS_X = c_int(0)			
        self.NAV_AXIS_Y = c_int(1)		
        self.NAV_AXIS_Z = c_int(2)          
        self.NAV_BUT_LEFT = c_int(0)			# bit 0 value 1 Left  SpaceNav Button
        self.NAV_BUT_RIGHT = c_int(1)			# bit 1 value 2 Right SpaceNav Button
        
        
        ## Volume Capture Modes
        
        self.VOLCAP_OFF = c_int(0)             # no capture
        self.VOLCAP_FRAME_PLY = c_int(1)       # single frame as .ply
        self.VOLCAP_FRAME_PNG = c_int(2)       # single frame as .png
        self.VOLCAP_FRAME_REC = c_int(3)       # single frame as .rec format (a Voxon proprietary file format can play back in voxieOS)
        self.VOLCAP_VIDEO_REC = c_int(4)       # multi frame / video in .rec format (records keystrokes, primitives and link to 3D models) 
        self.VOLCAP_FRAME_VCB = c_int(5)       # single frame as .vcb a 'volumetric screen  shot'. A Voxon proprietary file format can play back in voxieOS
        self.VOLCAP_VIDEO_VCBZIP = c_int(6)    # vcb video as a zip file for  animation playback  
        
        
        ## Menu Functions
        
        self.MENU_TEXT = c_int(0)							        #  text (decoration only)
        self.MENU_LINE = c_int(1)							        #  line (decoration only)
        self.MENU_BUTTON_MIDDLE = c_int(2)                          #  push button in the middle of a group
        self.MENU_BUTTON_LAST  = c_int(4)	                        #  push button last in the group
        self.MENU_BUTTON_FIRST = c_int(3)	                        #  push button first in the group
        self.MENU_BUTTON_FIRST = c_int(3)	                        #  push button first in the group
        self.MENU_BUTTON_SINGLE = c_int(5)	                        #  push button single button
        self.MENU_HSLIDER = c_int(6)			                    #  horizontal slider
        self.MENU_VSLIDER = c_int(7)						        #  vertical slider
        self.MENU_EDIT = c_int(8)							        #  edit text box
        self.MENU_EDIT_DO = c_int(9)						        #  edit text box + activates next iteself.M on 'Enter'
        self.MENU_TOGGLE = c_int(10)						        #  coself.Mbo box (w/o the drop down). Useful for saving space in dialog.
        											                    #  Specify multiple strings in 'st' using \r as separator.
        self.MENU_PICKFILE = c_int(11)						        #  File selector. Specify type in 2nd strig. Exaself.Mple "Browse\r*.kv6"        
        
        
        ## define Key types
        
        self.KEY_Empty = c_int(0x00)				# 0x00 Empty state 
        self.KEY_Escape = c_int(0x01)				# 0x01 'Esc' / Escape key
                   
        self.KEY_1 = c_int(0x02)					# c_int(0x02 Number 1 key
        self.KEY_2 = c_int(0x03)					# c_int(0x03 Number 2 key
        self.KEY_3 = c_int(0x04)					# c_int(0x04 Number 3 key
        self.KEY_4 = c_int(0x05)					# c_int(0x05 Number 4 key
        self.KEY_5 = c_int(0x06)					# c_int(0x06 Number 5 key
        self.KEY_6 = c_int(0x07)					# c_int(0x07 Number 6 key
        self.KEY_7 = c_int(0x08)					# c_int(0x08 Number 7 key
        self.KEY_8 = c_int(0x09)					# c_int(0x09 Number 8 key
        self.KEY_9 = c_int(0x0A)					# c_int(0x0A Number 9 key
        self.KEY_0 = c_int(0x0B)					# c_int(0x0B Number 0 key
        
        # Alpha Keys
        
        self.KEY_A = c_int(0x1E)					# c_int(0x1E 'A' key
        self.KEY_B = c_int(0x30)					# c_int(0x30 'B' key
        self.KEY_C = c_int(0x2E)					# c_int(0x2E 'C' key
        self.KEY_D = c_int(0x20)					# c_int(0x20 'D' key
        self.KEY_E = c_int(0x12)					# c_int(0x12 'E' key
        self.KEY_F = c_int(0x21)					# c_int(0x21 'F' key
        self.KEY_G = c_int(0x22)					# c_int(0x22 'G' key
        self.KEY_H = c_int(0x23)					# c_int(0x23 'H' key
        self.KEY_I = c_int(0x17)					# c_int(0x17 'I' key
        self.KEY_J = c_int(0x24)					# c_int(0x24 'J' key
        self.KEY_K = c_int(0x25)					# c_int(0x25 'K' key
        self.KEY_L = c_int(0x26)					# c_int(0x26 'L' key
        self.KEY_M = c_int(0x32)					# c_int(0x32 'M' key
        self.KEY_N = c_int(0x31)					# c_int(0x31 'N' key
        self.KEY_O = c_int(0x18)					# c_int(0x18 'O' key
        self.KEY_P = c_int(0x19)					# c_int(0x19 'P' key
        self.KEY_Q = c_int(0x10)					# c_int(0x10 'Q' key
        self.KEY_R = c_int(0x13)					# c_int(0x13 'R' key
        self.KEY_S = c_int(0x1F)					# c_int(0x1F 'S' key
        self.KEY_T = c_int(0x14)					# c_int(0x14 'T' key
        self.KEY_U = c_int(0x16)					# c_int(0x16 'U' key
        self.KEY_V = c_int(0x2F)					# c_int(0x2F 'V' key
        self.KEY_W = c_int(0x11)					# c_int(0x11 'W' key
        self.KEY_X = c_int(0x2D)					# c_int(0x2D 'X' key
        self.KEY_Y = c_int(0x15)					# c_int(0x15 'Y' key
        self.KEY_Z = c_int(0x2C)					# c_int(0x2C 'Z' key
        
        # Special Keys
        
        self.KEY_Alt_Left = c_int(0x38)				# c_int(0x38 Left Alt
        self.KEY_Alt_Right = c_int(0xB8)			# c_int(0xB8 Right Alt  
        self.KEY_Backspace = c_int(0x0E)			# c_int(0x0E Backspace
        self.KEY_CapsLock = c_int(0x3A)				# c_int(0x3A Capslock
        self.KEY_Comma = c_int(0x33)				# c_int(0x33 Comma
        self.KEY_Control_Left = c_int(0x1D)			# c_int(0x1D Left Control
        self.KEY_Control_Right = c_int(0x9D)		# c_int(0x9D Right Control
        self.KEY_Delete = c_int(0xD3)				# c_int(0xD3 Delete
        self.KEY_Forward_Slash = c_int(0x35)		# c_int(0x35 Divide
        self.KEY_Full_Stop = c_int(0x34)			# c_int(0x34 Fullstop / Greater Than
        self.KEY_End = c_int(0xCF)					# c_int(0xCF End
        self.KEY_Enter = c_int(0x1C)				# c_int(0x1C Enter
        self.KEY_Equals = c_int(0x0D)				# c_int(0x0D Equals
        self.KEY_Home = c_int(0xC7)					# c_int(0xC7 Home
        self.KEY_Insert = c_int(0xD2)				# c_int(0xD2 Insert
        self.KEY_Minus = c_int(0x0C)				# c_int(0x0C Minus / Dash 
        self.KEY_Numlock = c_int(0xC5)				# c_int(0xC5 Numlock
        self.KEY_Page_Down = c_int(0xD1)			# c_int(0xD1 Page Down
        self.KEY_Page_Up = c_int(0xC9)				# c_int(0xC9 Page Up
        self.KEY_Pause = c_int(0x45)				# c_int(0x45 Pause
        self.KEY_Print_Screen = c_int(0xB7)			# c_int(0xB7 Print_Screen
        self.KEY_Secondary_Action = c_int(0xDD)		# c_int(0xDD Secondary Action (not on all keyboards)
        self.KEY_Semicolon = c_int(0x27)			# c_int(0x27 Semicolon / Less Than
        self.KEY_ScrollLock = c_int(0x46)			# c_int(0x46 Scrolllock
        self.KEY_Shift_Left = c_int(0x2A)			# c_int(0x2A Left Shift
        self.KEY_Shift_Right = c_int(0x36)			# c_int(0x36 Right Shift
        self.KEY_Single_Quote = c_int(0x28)			# c_int(0x28 Single Quote / Double Quote
        self.KEY_Space_Bar = c_int(0x39)			# c_int(0x39 Spacebar
        self.KEY_Square_Bracket_Open = c_int(0x1A)	# c_int(0x1A Open Square Bracket 
        self.KEY_Square_Bracket_Close = c_int(0x1B) # c_int(0x1B Closed Square Bracket
        self.KEY_Tab = c_int(0x0F)					# c_int(0x0F Tab
        self.KEY_Tilde = c_int(0x29)				# c_int(0x29 Tilde
        self.KEY_Back_Slash = c_int(0x2B)			# c_int(0x2B BackSlash (Used by VX1 to open menu)
        
        # Function Keys
        
        self.KEY_F1 = c_int(0x3B)					# c_int(0x3B F1 function key
        self.KEY_F2 = c_int(0x3C)					# c_int(0x3C F2 function key
        self.KEY_F3 = c_int(0x3D)					# c_int(0x3D F3 function key
        self.KEY_F4 = c_int(0x3E)					# c_int(0x3E F4 function key
        self.KEY_F5 = c_int(0x3F)					# c_int(0x3F F5 function key
        self.KEY_F6 = c_int(0x40)					# c_int(0x40 F6 function key
        self.KEY_F7 = c_int(0x41)					# c_int(0x41 F7 function key
        self.KEY_F8 = c_int(0x42)					# c_int(0x42 F8 function key
        self.KEY_F9 = c_int(0x43)					# c_int(0x43 F9 function key
        self.KEY_F10 = c_int(0x44)					# c_int(0x44 F10 function key
        self.KEY_F11 = c_int(0x57)					# c_int(0x57 F11 function key
        self.KEY_F12 = c_int(0x58)					# c_int(0x58 F12 function key
        
        # Numpad
        
        self.KEY_NUMPAD_0 = c_int(0x52)				# c_int(0x52 Numpad 0 key
        self.KEY_NUMPAD_1 = c_int(0x4F)				# c_int(0x4F Numpad 1 key
        self.KEY_NUMPAD_2 = c_int(0x50)				# c_int(0x50 Numpad 2 key	
        self.KEY_NUMPAD_3 = c_int(0x51)				# c_int(0x51 Numpad 3 key
        self.KEY_NUMPAD_4 = c_int(0x4B)				# c_int(0x4B Numpad 4 key
        self.KEY_NUMPAD_5 = c_int(0x4C)				# c_int(0x4C Numpad 5 key
        self.KEY_NUMPAD_6 = c_int(0x4D)				# c_int(0x4D Numpad 6 key
        self.KEY_NUMPAD_7 = c_int(0x47)				# c_int(0x47 Numpad 7 key
        self.KEY_NUMPAD_8 = c_int(0x48)				# c_int(0x48 Numpad 8 key
        self.KEY_NUMPAD_9 = c_int(0x49)				# c_int(0x49 Numpad 9 key
        self.KEY_NUMPAD_Decimal = c_int(0x53)		# c_int(0x53 Numpad decimal key
        self.KEY_NUMPAD_Divide = c_int(0xB5)		# c_int(0xB5 Numpad divide key
        self.KEY_NUMPAD_Multiply = c_int(0x37)		# c_int(0x37 Numpad multiply key
        self.KEY_NUMPAD_Minus = c_int(0x4A)			# c_int(0x4A Numpad minus key
        self.KEY_NUMPAD_Plus = c_int(0x4E)			# c_int(0x4E Numpad plus key
        self.KEY_NUMPAD_Enter = c_int(0x9C)			# c_int(0x9C Numpad enter key
        
        # Arrow Keys
        
        self.KEY_Arrow_Right = c_int(0xCD)			# c_int(0xCD Arrow Right
        self.KEY_Arrow_Left = c_int(0xCB)			# c_int(0xCB Arrow Left
        self.KEY_Arrow_Up = c_int(0xC8)				# c_int(0xC8 Arrow Up
        self.KEY_Arrow_Down = c_int(0xD0)			# c_int(0xD0 Arrow Down
 
        
## uncomment these to test runtime....
if TEST == True:
    vxpy = VoxiePy()
    vxpy.Test()
    vxpy.Uninit()  


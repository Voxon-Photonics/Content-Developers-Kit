# -*- coding: utf-8 -*-
"""
Created on Mon Dec 19 17:14:04 2022

@author: Mat
"""

## example on how to draw some primatives on the Voxon Display 


from VxPyDataTypes import  * # import various VX related data types
from ctypes import *         # ctypes is the python library that works with C based DLLs
from VoxiePy import *           # the core library to  allow pyhton to work with the Voxiebox.DLL
       
import math
import random
import time



class VXApp(VoxiePy):
    
  
   
    def __init__(self, VoxiePy):
        vx = VoxiePy ## shorter name for the VX runtime
        vxd = vx.vxDLL ## shorter name to access the DLL part of the VxPy object
        
        pos = point3d(0,0,0)
        rVec = point3d(1,0,0)
        dVec = point3d(0,1,0)
        fVec = point3d(0,0,1)
        fillmode = 1
                
        ## setup variables to draw a polygon
        ## creating an array of c_type structs can be tricky so look at this code closely
        vt = ((poltex_t) * 4)() ## this is how we can define an array of struccts
        
        vt[0].x =-0.4 
        vt[0].y =-0.8 
        vt[0].z =-0.2 
        vt[0].col = 0xff0000
        vt[1].x =-0.4
        vt[1].y =-0.4
        vt[1].z =+0.2
        vt[1].col = 0x00ff00
        vt[2].x =+0.4
        vt[2].y =-0.8
        vt[2].z =+0.2
        vt[2].col = 0x0000ff
        vt[3].x =+0.4
        vt[3].y =-0.4
        vt[3].z =-0.2
        vt[3].col = 0xff00ff
        vtSize = 4
        
        ## how to define a C array
        mesh = (c_int * 16)(0,1,2,-1,1,0,3,-1,2,1,3,-1,0,2,3,-1) ## first  () is the datatype  the second () is the initial value
        meshSize = 16
        


        try:
            while (vx.Breath() == 0): ## the breath loop -- this is the main loop for your program pres the 'Esc' key to quit the loop
              
              
                # input functions press 0,1,2,3 to set fillmode
                if vxd.voxie_keystat(vx.KEY_1) : ## keyboard press 1 - get the scan codes from vx.KEY
                    fillmode = 1
                if vxd.voxie_keystat(vx.KEY_2) : ##  keyboard press 2
                    fillmode = 2     
                if vxd.voxie_keystat(vx.KEY_3) : ## keyboard press 3
                    fillmode = 3 
                if vxd.voxie_keystat(vx.KEY_0) : ## keyboard press 4
                    fillmode = 0  
 
   
                vx.StartFrame() ## start drawing the frame - put all draw calls between the frame functions
                
                
                ## making a draw mesh texture call is a little tricky the c_types arrays we made aren't passed in as pointers, and the filename string is a binary string
                vxd.voxie_drawmeshtex(byref(vx.vf),c_char_p(b"0"),vt,4,mesh,16,16+fillmode,0x404040)
                
                
                vxd.voxie_drawbox(byref(vx.vf), -0.9, 0, -0.4, -0.5, 0.4,0,fillmode, 0xff0000 )
        
                pos.x = -0.4
                pos.y = 0
                pos.z = -0.4
                rVec.x = 0.4
                rVec.y = 0
                rVec.z = 0
                dVec.x = 0
                dVec.y = 0.4
                dVec.z = 0
                fVec.x = 0
                fVec.y = 0
                fVec.z = 0.4    
                
                vxd.voxie_drawcube(byref(vx.vf), byref(pos), byref(rVec), byref(dVec), byref(fVec), fillmode, 0x00ff00)
                
                pos.x = 0.3
                pos.y = 0.2
                pos.z = -0.2
            
                rVec.x = 0.4
                rVec.y = 0
                rVec.z = 0
                dVec.x = 0
                dVec.y = 0.4
                dVec.z = 0
                fVec.x = 0 
                fVec.y = 0
                fVec.z = 0.4
                               
                vxd.voxie_drawspr(byref(vx.vf), c_char_p(b"assets/cube.obj"), byref(pos), byref(rVec), byref(dVec), byref(fVec),  0x0000ff)
              
                vxd.voxie_drawsph(byref(vx.vf), 0.8, 0.2, -0.2, .15,fillmode, 0xffff00 )
                
                vxd.voxie_drawlin(byref(vx.vf), -0.8, -0.2, -0.4, 0.8, -0.2, 0.4,0x00ffff )
                
                vxd.voxie_drawcone(byref(vx.vf), -0.8, 0.7, 0.2, 0.1, 0.8, 0.7, -0.2, 0.2, fillmode, 0xff00ff )
                                                       
                vx.ShowStats(10, 50)
                
                ## use c_char_p and pass the string as a binary to create a char *
                vxd.voxie_debug_print6x8(30, 128, 0x00ff80, -1, c_char_p(b"Polygon and Primitive shape demo press 0 - 3 to change fillmode")); 
            
                # have to convert varibles to a string before you send it to be shown on the secondary screen            
                msg = "Fillmode : {}".format(int(fillmode))
                msg_b = bytes(msg,'UTF-8') ## converts the msg to a binary readable format by the voxiescreen
                vxd.voxie_debug_print6x8(30, 148, 0x00ff80, -1, msg_b); 
                
                vxd.voxie_debug_print6x8(30, 168, 0xffff80, -1, c_char_p(b"what is being shown on the screen from top left to bottom right:\nvoxie_drawmeshtex\nvoxie_drawlin\nvoxie_drawbox\nvoxie_drawcube\nvoxie_drawspr (cube.obj)\nvoxie_drawsph\nvoxie_drawcone ")); 

                         
                vx.EndFrame() ## end drawing the frame - put all draw calls between the frame functions
                
                

    
            vx.Uninit() ## clean up and exit the program
        except Exception as e: ## error handling, close the VX window, print the error and wait 5 seconds
            print("\n****** VxApp Error *****\n")
            print("Error in VxApp Details :")
            print(e)
            print("\n\n\nNow unloading VxPy...")
            vx.EndFrame() ## end drawing the frame - put all draw calls between the frame functions
            vx.Uninit() ## clean up and exit the program
            time.sleep(5)
   





try:                    # Surround the VoxiePy in a try catch to gracefully close the VoxieBox Window....
    voxiePy = VoxiePy()       # Create a VoxiePy Object 
    vxApp = VXApp(voxiePy) # Create a VoxiePy App Object and pass in the VoxiePy runtime
except Exception as e:
        print("\n******Vx App Launch Error *****\n")
        print("VxPy or VxApp failed to load:")
        print(e)
        print("\n\n\n")

        voxiePy.Uninit()
        time.sleep(5)



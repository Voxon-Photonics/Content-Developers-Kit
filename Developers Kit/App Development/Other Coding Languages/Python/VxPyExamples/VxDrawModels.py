# -*- coding: utf-8 -*-
"""
Created on Wed Jan 11 17:15:31 2023

Use VxPy to draw some 3d Models - uses the blenders 'Suzanne.obj' model so make sure that model file is included with your script

it will draw the Suzanne obj in three different positions in three different colors

@author: Mat

"""



from VxPyDataTypes import  * # import various VX related data types
from ctypes import *         # ctypes is the python library that works with C based DLLs
from VxPy import *           # the core library to  allow pyhton to work with the Voxiebox.DLL
       


import VxPyTools as vxt      # VxPyTools contains various helper functions to assist with VX development

import math
import random
import time

debug = 1 
modelFound = int(0)


class VXApp(VxPy):
    
   
    def __init__(self, VxPy):
        try:
            vx = VxPy            # shorter name for the VX runtime
            vxd = vx.vxDLL       # shorter name to access VX DLL functions 
    
            while (vx.Breath() == 0):                                                # the breath loop -- this is the main loop for your program press the 'Esc' key to quit the loop
            
                ### Input 
                
                ### Update
                
                
                vx.StartFrame()                                                      # start processing the volumetric frame - place all draw calls between the frame functions
               
                ### Draw  
               
                ## define positions for the models
                pp = point3d(0,0,0)
                pp2 = point3d(.5,0,0)
                pp3 = point3d(-.5,0,0)
    
                ## define the right, down and forward vectors this is used for rotation
                rr = point3d(1,0,0)
                dd = point3d(0,1,0)
                ff = point3d(0,0,1)
    
                ## params are  ( voxieframe, filepath / name, position, r Vector, down Vector, forward Vector, color hex (0x404040 to use the model's color) )
                modelFound = vxd.voxie_drawspr(byref(vx.vf), b"assets\suzanne.obj", byref(pp3), byref(rr), byref(dd), byref(ff), 0xff0000)                    
    
                modelFound = vxd.voxie_drawspr(byref(vx.vf), b"assets\suzanne.obj", byref(pp), byref(rr), byref(dd), byref(ff), 0x00ff00)
    
                modelFound = vxd.voxie_drawspr(byref(vx.vf), b"assets\suzanne.obj", byref(pp2), byref(rr), byref(dd), byref(ff), 0xffff00) 
               
                ### Debug
                
                if modelFound == 1:
                        vx.vxDLL.voxie_debug_print6x8(10, 300, 0xffffff,-1, c_char_p(b"Suzanne.obj model found"))
                else :
                    vx.vxDLL.voxie_debug_print6x8(10, 200, 0xff0000,-1, c_char_p(b"Error Suzanne.obj model not found ensure that Suzanne.obj is in the assets subfolder with this Pyhton script"))
                
                
                if debug == 1 :
                
                        vx.ShowStats(20, 600)
    
                vx.EndFrame()                                                        # process all draw calls and create the volumetric frame - ensure all draw calls between the frame functions
    
            vx.Uninit()                                                              # clean up and exit VX application
    
        except Exception as e: ## error handling, close the VX window, print the error and wait 5 seconds
            print("\n****** VxApp Error *****\n")
            print("Error in VxApp Details :")
            print(e)
            print("\n\n\nNow unloading VxPy...")
            vx.EndFrame() ## end drawing the frame - put all draw calls between the frame functions
            vx.Uninit() ## clean up and exit the program
            time.sleep(5)
       

try:                    # Surround the VxPy in a try catch to gracefully close the VoxieBox Window....
    vxpy = VxPy()       # Create a VxPy Object 
    vxApp = VXApp(vxpy) # Create a VxPy App Object and pass in the VxPy runtime
except Exception as e:
        print("\n******Vx App Launch Error *****\n")
        print("VxPy or VxApp failed to load:")
        print(e)
        print("\n\n\n")

        vxpy.Uninit()
        time.sleep(5)    

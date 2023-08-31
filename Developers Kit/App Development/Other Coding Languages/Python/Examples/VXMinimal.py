# -*- coding: utf-8 -*-
"""
Created on Fri Dec 16 10:59:35 2022

Minimal setup for a VxPy application. 

Use this file as a template to build your own projects

@author: Mathew Vecchio / Ready Wolf for Voxon 
"""



from VxPyDataTypes import  * # import various VX related data types
from ctypes import *         # ctypes is the python library that works with C based DLLs
from VoxiePy import *           # the core library to  allow pyhton to work with the Voxiebox.DLL
       
import time

class VXApp(VoxiePy):
    
   
    def __init__(self, VoxiePy):
        try:
            vx = VoxiePy            # shorter name for the VX runtime
    
            while (vx.Breath() == 0):                                                # the breath loop -- this is the main loop for your program press the 'Esc' key to quit the loop
            
                vx.StartFrame()                                                      # start processing the volumetric frame - place all draw calls between the frame functions
               
                vx.EndFrame()                                                        # process all draw calls and create the volumetric frame - ensure all draw calls between the frame functions
    
            vx.Uninit()                                                              # clean up and exit VX application
    
        except Exception as e: ## error handling, close the VX window, print the error and wait 5 seconds
            print("\n****** VxApp Error *****\n")
            print("Error in VxApp Details :")
            print(e)
            print("\n\n\nNow unloading VoxiePy...")
            vx.EndFrame() ## end drawing the frame - put all draw calls between the frame functions
            vx.Uninit() ## clean up and exit the program
            time.sleep(5)
   

try:                    # Surround the VxPy in a try catch to gracefully close the VoxieBox Window....
    voxiePy = VoxiePy()       # Create a VxPy Object 
    vxApp = VXApp(voxiePy) # Create a VxPy App Object and pass in the VxPy runtime
except Exception as e:
        print("\n******Vx App Launch Error *****\n")
        print("VxPy or VxApp failed to load:")
        print(e)
        print("\n\n\n")

        voxiePy.Uninit()
        time.sleep(5)    
   
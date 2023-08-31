# -*- coding: utf-8 -*-
"""
Created on Tue Dec 20 11:43:22 2022

@author: Matthew Vecchio for Voxon

# Little demo of rotating cubes for a fun effect

"""




from VxPyDataTypes import  *
from ctypes import Structure, c_float, c_int, c_double, c_void_p, c_char_p, WinDLL, byref, POINTER
from VxPy import *

       
import math
import random
## VxPyTools contains various helper functions to assist with VX development
import VxPyTools as vxt 



class VXApp(VxPy):
    
   
    def __init__(self, VxPy):
        try:
            vx = VxPy ## shorter name for the VX runtime
            vxd = vx.vxDLL ## shorter name to access the DLL part of the VxPy object
        
        
        
            pos = point3d(0,0,0)
            rVec = point3d(1,0,0)
            dVec = point3d(0,1,0)
            fVec = point3d(0,0,1)
            
            scale = 2
            rr = point3d()
            dd = point3d()
            ff = point3d()
            colNo = 0
            
            fillmode = 1
    
   
            while (vx.Breath() == 0): ## the breath loop -- this is the main loop for your program pres the 'Esc' key to quit the loop
            
                # input functions press 0,1,2,3 to set fillmode
                if vxd.voxie_keystat(vx.KEY_1) : ## keyboard press 1 
                    fillmode = 1
                if vxd.voxie_keystat(vx.KEY_2) : ##  keyboard press 2
                    fillmode = 2     
                if vxd.voxie_keystat(vx.KEY_3) : ## keyboard press 3
                    fillmode = 3 
                if vxd.voxie_keystat(vx.KEY_0) : ## keyboard press 4
                    fillmode = 0      
                    
                vx.StartFrame() ## start drawing the frame - put all draw calls between the frame functions
                                
                 
                # draw cube voxie_drawcube (vf, pos*, rVec*, dVec*, fVec*, fill, col)
                scale = 0.2 + (math.cos(vx.time) *.15)
                rr = point3d(rVec.x * scale, rVec.y * scale, rVec.z * scale)
                dd = point3d(dVec.x * scale, dVec.y * scale, dVec.z * scale)
                ff = point3d(fVec.x * scale, fVec.y * scale, fVec.z * scale)
                
                vxt.RotVex(1 * vx.deltaTime, rVec, fVec)
                
                vxt.RotVex(-2 * vx.deltaTime, dVec, fVec)
            
                vxt.RotVex(1 * vx.deltaTime, fVec, rVec)
            
            
                j = 0       
                k = 0
                l = 0
                pos.y = -0.9
                
                for i in range(500): 
                    
                    j += 1    
                    pos.x = -0.9 + ( 0.3 * j)
                    if pos.x > vx.vw.aspx : 
                        j = 0
                        k += 1
                    
                    pos.y = -0.9 + ( 0.3 * k)
                    if pos.y > vx.vw.aspy :
                        j = 0
                        k = 0
                        l += 1
                    pos.z = 0.4 + ( -0.15 * l)
                    
                    if (pos.z > vx.vw.aspz) :
                        
                        
                        break
                    
                
                    vxd.voxie_drawcube(byref(vx.vf), byref(pos), byref(rr), byref(dd), byref(ff), fillmode, vxt.RANDOM_COLOR[colNo])


       
                colNo += 1
                if (colNo > len(vxt.RANDOM_COLOR) - 1) :
                    colNo = 0
                
                vx.ShowStats(10, 60)
                vxd.voxie_debug_print6x8(10, 100, 0x00ffff,0x003333, c_char_p(b"Cube Demos -> press 0 - 3 to change fillmode"))         
                vx.EndFrame() ## end drawing the frame - put all draw calls between the frame functions
                
                

    
            vx.Uninit() ## clean up and exit the program
        except Exception as e: 
            print("## Error in VXapp ##\n")
            print(e)
            print("\n## End of Error - Attempting to free DLL ##")
            vx.Uninit()
 

try:                    # Surround the VxPy in a try catch to gracefully close the VoxieBox Window....
    vxpy = VxPy()       # Create a VxPy Object 
    vxApp = VXApp(vxpy) # Create a VxPy App Object and pass in the VxPy runtime
except Exception as e:
        print("\n******VxApp ERROR *****\n")
        print("Error in VxApp Details :")
        print(e)
        print("\n\n\n")
        vxpy.Uninit()


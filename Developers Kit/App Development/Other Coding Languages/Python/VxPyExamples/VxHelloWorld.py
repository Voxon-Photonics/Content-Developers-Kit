# -*- coding: utf-8 -*-
"""
Created on Wed Oct 28 15:30:05 2020

@author: Mat Vex
"""

## Python 'Hello World' example running VoxieBox.DLL

from VxPyDataTypes import  *
from ctypes import *
from VxPy import *
import VxPyTools as vxt ## various helper functions for VxPy development





## position of text
pos = point3d(-.9,-.7,0)
rvec = point3d(.1,0,0) ## the right vector sets the font width 
dvec = point3d(0,.1,0) ## the down vector sets the font height
fvec = point3d(0,0,.1)


class VXApp(VxPy):
   

   
    def __init__(self, VxPy):
        
        vx = VxPy ## shorter name for the VX runtime
        
        try:
          
                   
            while (vx.Breath() == 0):
    
               
    
                    ## start frame - all draw calls after this line
                    vx.StartFrame()
                    
                 
                    ## to pass for pointers in Python / c_types use the byref() function
                    vx.vxDLL.voxie_printalph(vx.vf,byref(pos), byref(rvec), byref(dvec), 0xff00ff, c_char_p(b"Hello World"))
    
                    ## to pass in variables you'll have to work out the formating of variables BEFORE you send it to the DLL 
                    msg = "\nDelta : {:.3}".format(float(vx.deltaTime))        
                    msg_b = bytes(msg,'UTF-8')
                    vx.vxDLL.voxie_printalph(vx.vf,byref(pos), byref(rvec), byref(dvec), 0x00ff00,  c_char_p(msg_b))
                    
                    ## to pass in variables you'll have to work out the formating of variables BEFORE you send it to the DLL 
                    msg = "\n\nRuntime : {:.3}".format(float(vx.time))
                    msg_b = bytes(msg,'UTF-8')
                    vx.vxDLL.voxie_printalph(vx.vf,byref(pos), byref(rvec), byref(dvec), 0xffff00,  c_char_p(msg_b))
                    
                    ## write to the touchscreen
                    vx.vxDLL.voxie_debug_print6x8(10, 300, 0xffffff,-1, c_char_p(b"Hello World ! Touch Screen"))
                    
    
                    ## draw a border around the view
                    vxt.DrawBorder(VxPy, 0x00ffff) 
                  
                    ## uses VXTools for rotating the vectors
                    vxt.RotVex(1 * vx.deltaTime,dvec, fvec)

                    ## show stats for on touch screen
                    vx.ShowStats(5,60)
                    
                    ## end frame - No more VX draw calls after this line
                    vx.EndFrame()
          
            vx.Uninit()

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



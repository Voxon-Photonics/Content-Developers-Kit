# -*- coding: utf-8 -*-
"""
Created on Thu Jan 12 15:30:05 2023


A keyboard and mouse input test allows the user to move the cursor (drawn as a sphere) using the arrow keys or mouse

@author: Matthew Vecchio / Ready Wolf for Voxon
"""

## Python running VoxieBox.DLL

from VxPyDataTypes import  *
from ctypes import *
from VxPy import *
       

import time
## VxPyTools contains various helper functions to assist with VX development
import VxPyTools as vxt 
    


curPos = point3d()
curSize = 0.2
movspeed = 1
obutState = 0
mouseSensitivity = 0.001


class VXApp(VxPy):
    
   
    def __init__(self, VxPy):
        try:
            vx = VxPy ## shorter name for the VX runtime
    
            curCol = int(0xffffff)
            curFill = 1
            while (vx.Breath() == 0):       
    
                    #### Input
               
                    ## check for keyboard updates voxie_keystat return 1 if pressed down on that update and (3 on continue press)
                    
                    if vx.vxDLL.voxie_keystat(vx.KEY_Arrow_Up) :
                        curPos.y -= movspeed * vx.deltaTime
    
                    if vx.vxDLL.voxie_keystat(vx.KEY_Arrow_Down) :
                        curPos.y += movspeed * vx.deltaTime		
    
                    if vx.vxDLL.voxie_keystat(vx.KEY_Arrow_Left) :
                        curPos.x -= movspeed * vx.deltaTime
    
                    if vx.vxDLL.voxie_keystat(vx.KEY_Arrow_Right) :
                        curPos.x += movspeed * vx.deltaTime	
                    
                    if vx.vxDLL.voxie_keystat(vx.KEY_Z) :
                        curPos.z += (movspeed *  .5) * vx.deltaTime
    
                    if vx.vxDLL.voxie_keystat(vx.KEY_A) :
                        curPos.z -= (movspeed *  .5) * vx.deltaTime	
                    
                    ## press space to randomise color 
                    if vx.vxDLL.voxie_keystat(vx.KEY_Space_Bar) :
                        curCol = vxt.RandomCol() 
    
                    ## press x to toggle fill mode  
                    if vx.vxDLL.voxie_keystat(vx.KEY_X) == 1 :
                        curFill = 1 if curFill == 0 else 0 ## ternary condition to switch between cursor fill mode 1 or 0 
    
    
    
                    ## check for mouse movement - the  voxie_input_t handles this                    
                    if vx.in_t.dmousx != 0:
                        curPos.x += (vx.in_t.dmousx * mouseSensitivity)
                    if vx.in_t.dmousy != 0:
                        curPos.y += (vx.in_t.dmousy * mouseSensitivity)
                    if vx.in_t.dmousz != 0:
                        curPos.z += (vx.in_t.dmousz * (mouseSensitivity * .5))
    
                    ## how to handle button presses
                    
                    
                    ## button states are processed in bit as a mouse has only 2 buttons it works like this
                    #  look at the bstat variable stands for button state. the obstat is the previous frame's button state (use it to determine just pressed)
                    #  vx.in_t.bstat = 1  is left click only
                    #  vx.in_t.bstat = 2  is right click onlly
                    #  vx.in_t.bstat = 3  is both buttons clicked
                    
                    ## if the left button is pressed down randomise cursor color
                    if vx.in_t.bstat == 1 or vx.in_t.bstat == 3 :
                        curCol = vxt.RandomCol()
                        
                    ## compare with the old button state to make a just pressed call
                    if (vx.in_t.bstat == 2 or vx.in_t.bstat == 3) and vx.in_t.obstat <= 1 :
                       curFill = 1 if curFill == 0 else 0 ## ternary condition to switch between cursor fill mode 1 or 0 
    
    
    
                    vx.StartFrame()                                                      # start processing the volumetric frame - place all draw calls between the frame functions
          
                         
                    ## draw the cursor at the position
                    vx.vxDLL.voxie_drawsph(vx.vf, curPos.x,curPos.y,curPos.z,curSize,curFill,curCol)
                       
                    ##  check if our curPos is out of bounds.
                    if curPos.x > vx.vw.aspx :
                        curPos.x = vx.vw.aspx - 0.001
                    if curPos.x < -vx.vw.aspx :
                        curPos.x = -vx.vw.aspx + 0.001
                        
                    if curPos.y > vx.vw.aspy :
                        curPos.y = vx.vw.aspy - 0.001
                    if curPos.y < -vx.vw.aspy :
                        curPos.y = -vx.vw.aspy + 0.001
                        
                    if curPos.z > vx.vw.aspz :
                        curPos.z = vx.vw.aspz - 0.001
                    if curPos.z < -vx.vw.aspz :
                        curPos.z = -vx.vw.aspz + 0.001
    
    
                    ## draw box around  the outside of the box
                    vx.vxDLL.voxie_drawbox(byref(vx.vf), -vx.vw.aspx + 0.001, -vx.vw.aspy + 0.001, -vx.vw.aspz, vx.vw.aspx - 0.001, vx.vw.aspy - 0.001,	vx.vw.aspz, 1, 0xffffff);
        
                    ## end frame - No more VX draw calls after this  line
                    
                    ## write out the cursor position on screen
                    ## to pass in variables you'll have to work out the formating of variables BEFORE you send it to the DLL 
                    msg = "Input Test use the mouse of the arrow keys, space, a z and x keys to move Cursor!\nCurs Pos X:{:.3}, Y:{:.3}, Z:{:.3}".format(curPos.x, curPos.y, curPos.z) 
                    msg_b = bytes(msg,'UTF-8')
                    vx.vxDLL.voxie_debug_print6x8(20, 100, 0xff0033,0x330011, c_char_p(msg_b))
                    
                    vx.ShowStats(20, 600)
    
                    vx.EndFrame()     
          
            vx.Uninit()
    
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
      
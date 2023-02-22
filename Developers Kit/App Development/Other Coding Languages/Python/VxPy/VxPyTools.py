# -*- coding: utf-8 -*-
"""
Created on Mon Dec 19 12:54:22 2022

This is a helper module which adds some useful functions for your VxPy development

@author: Mat


"""

from VxPyDataTypes import  *
from VxPy import *

import math
import random
# Useful helper functions for the DLL




# collection bold colors chosen for a VX1. This is what VoxieBox::randomCol() draws from. Use with VoxieBox::randomCol()
RANDOM_COLOR = [ 0xFF00EE, 0xFF00DD, 0xFF00CC, 0xFF00BB, 0xFF00AA, 0xFF0099, 0xFF0088, 0xFF0077, 0xFF0066, 0xFF0055, 0xFF0044, 0xFF0033, 0xFF0022, 0xFF0011, 0xFF0000, # RED
0xFF1100, 0xFF2200, 0xFF3300, 0xFF4400, 0xFF5500, 0xFF6600, 0xFF7700, 0xFF8800, 0xFF9900, 0xFFAA00, 0xFFBB00, 0xFFCC00, 0xFFDD00, 0xFFEE00, 0xFFFF00, # YELLOW
0xEEFF00, 0xDDFF00, 0xCCFF00, 0xBBFF00, 0xAAFF00, 0x99FF00, 0x88FF00, 0x77FF00, 0x66FF00, 0x55FF00, 0x44FF00, 0x33FF00, 0x22FF00, 0x11FF00, 0x00FF00, # GREEN
0x00FF11, 0x00FF22, 0x00FF33, 0x00FF44, 0x00FF55, 0x00FF66, 0x00FF77, 0x00FF88, 0x00FF99, 0x00FFAA, 0x00FFBB, 0x00FFCC, 0x00FFDD, 0x00FFEE, 0x00FFFF, # CYAN
0x00EEFF, 0x00DDFF, 0x00CCFF, 0x00BBFF, 0x00AAFF, 0x0099FF, 0x0088FF, 0x0077FF, 0x0066FF, 0x0055FF, 0x0044FF, 0x0033FF, 0x0022FF, 0x0011FF, 0x0000FF, # BLUE
0x1100FF, 0x2200FF, 0x3300FF, 0x4400FF, 0x5500FF, 0x6600FF, 0x7700FF, 0x8800FF, 0x9900FF, 0xAA00FF, 0xBB00FF, 0xCC00FF, 0xDD00FF, 0xEE00FF, 0xFF00FF # MAGENTA
]


# collection bold colors chosen for a VX1 display. This is what VoxieBox::scrollCol() draws from. Use with VoxieBox::scrollCol()
PALETTE_COLOR = [
0xFF00EE, 0xFF00DD, 0xFF00CC, 0xFF00BB, 0xFF00AA, 0xFF0099, 0xFF0088, 0xFF0077, 0xFF0066, 0xFF0055, 0xFF0044, 0xFF0033, 0xFF0022, 0xFF0011, 0xFF0000, # RED
0xFF1100, 0xFF2200, 0xFF3300, 0xFF4400, 0xFF5500, 0xFF6600, 0xFF7700, 0xFF8800, 0xFF9900, 0xFFAA00, 0xFFBB00, 0xFFCC00, 0xFFDD00, 0xFFEE00, 0xFFFF00, # YELLOW
0xEEFF00, 0xDDFF00, 0xCCFF00, 0xBBFF00, 0xAAFF00, 0x99FF00, 0x88FF00, 0x77FF00, 0x66FF00, 0x55FF00, 0x44FF00, 0x33FF00, 0x22FF00, 0x11FF00, 0x00FF00, # GREEN
0x00FF11, 0x00FF22, 0x00FF33, 0x00FF44, 0x00FF55, 0x00FF66, 0x00FF77, 0x00FF88, 0x00FF99, 0x00FFAA, 0x00FFBB, 0x00FFCC, 0x00FFDD, 0x00FFEE, 0x00FFFF, # CYAN
0x00EEFF, 0x00DDFF, 0x00CCFF, 0x00BBFF, 0x00AAFF, 0x0099FF, 0x0088FF, 0x0077FF, 0x0066FF, 0x0055FF, 0x0044FF, 0x0033FF, 0x0022FF, 0x0011FF, 0x0000FF, # BLUE
0x1100FF, 0x2200FF, 0x3300FF, 0x4400FF, 0x5500FF, 0x6600FF, 0x7700FF, 0x8800FF, 0x9900FF, 0xAA00FF, 0xBB00FF, 0xCC00FF, 0xDD00FF, 0xEE00FF, 0xFF00FF # MAGENTA
]



# Rotate two point3d vectors a & b around their common plane, by angle expressed in radians.
def RotVex(angInRadians, a, b):

    c = math.cos(angInRadians)
    s = math.sin(angInRadians)
    f = a.x
   
  
    a.x = f * c + b.x * s
    b.x = b.x * c - f * s
    f = a.y;
    a.y = f * c + b.y * s
    b.y = b.y * c - f * s
    f = a.z;
    a.z = f * c + b.z * s
    b.z = b.z * c - f * s 

# Rotate two point3d vectors a & b around their common plane, by angle expressed in degrees.
def  RotVexD(angInDegrees, a, b):
    
    ang = (angInDegrees * math.pi) / 180
    c = math.cos(ang)
    s = math.sin(ang)
    f = a.x
   
  
    a.x = f * c + b.x * s
    b.x = b.x * c - f * s
    f = a.y;
    a.y = f * c + b.y * s
    b.y = b.y * c - f * s
    f = a.z;
    a.z = f * c + b.z * s
    b.z = b.z * c - f * s 
   
# Draw a border around the display set color in hex - currently only draws a box
def DrawBorder(VxPy, colorHex):
    
    
       VxPy.vxDLL.voxie_drawbox(byref(VxPy.vf), 
   					-VxPy.vw.aspx + 0.001, 
   					-VxPy.vw.aspy + 0.001, 
   					-VxPy.vw.aspz, 
   					VxPy.vw.aspx - 0.001, 
   					VxPy.vw.aspy - 0.001, 
   					VxPy.vw.aspz, 
   					1, colorHex);
       
# Retrieve a random color from the RANDOM_COLOR array defined above
def RandomCol():
        
    return RANDOM_COLOR[random.randint(0,len(RANDOM_COLOR) - 1)]



# quickly allows you to set a point3d
def p3dSet(x,y,z):
    return point3d (x,y,z )


# quickly allows you to scale a point3d
def p3dScale(point3d, scaleAmount):
    
    point3d.x * scaleAmount
    point3d.y * scaleAmount
    point3d.z * scaleAmount
        
    return point3d
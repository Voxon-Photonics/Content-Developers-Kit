#if 0
!if 1
#Visual C makefile:
customKey.exe: customKey.c voxiebox.h; cl /TP customKey.c /Ox /MT /link user32.lib
	del customKey.obj

!else

#GNU C makefile:
customKey.exe: customKey.c; gcc customKey.c -o customKey.exe -pipe -O3 -s -m64

!endif
!if 0
#endif

#include "voxiebox.h"
#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <conio.h>
#define PI 3.14159265358979323

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a, b) (((a) < (b)) ? (a) : (b))
#endif

static voxie_wind_t vw;
static voxie_frame_t vf; 

#define DEBUG 1

/**
* These are the scancodes standard keycodes for voxiebox.dll to read keyboard inputs. 
*
*
*/
enum Keys_scancodes
{
	KB_ = 0x00,						//!< 0x00 Empty state 
	KB_Escape = 0x01,				//!< 0x01 'Esc' / Escape ley

									// Numeric Keys

	KB_1 = 0x02,					//!< 0x02 Number 1 key
	KB_2 = 0x03,					//!< 0x03 Number 2 key
	KB_3 = 0x04,					//!< 0x04 Number 3 key
	KB_4 = 0x05,					//!< 0x05 Number 4 key
	KB_5 = 0x06,					//!< 0x06 Number 5 key
	KB_6 = 0x07,					//!< 0x07 Number 6 key
	KB_7 = 0x08,					//!< 0x08 Number 7 key
	KB_8 = 0x09,					//!< 0x09 Number 8 key
	KB_9 = 0x0A,					//!< 0x0A Number 9 key
	KB_0 = 0x0B,					//!< 0x0B Number 0 key

									// Alpha Keys

	KB_A = 0x1E,					//!< 0x1E 'A' key
	KB_B = 0x30,					//!< 0x30 'B' key
	KB_C = 0x2E,					//!< 0x2E 'C' key
	KB_D = 0x20,					//!< 0x20 'D' key
	KB_E = 0x12,					//!< 0x12 'E' key
	KB_F = 0x21,					//!< 0x21 'F' key
	KB_G = 0x22,					//!< 0x22 'G' key
	KB_H = 0x23,					//!< 0x23 'H' key
	KB_I = 0x17,					//!< 0x17 'I' key
	KB_J = 0x24,					//!< 0x24 'J' key
	KB_K = 0x25,					//!< 0x25 'K' key
	KB_L = 0x26,					//!< 0x26 'L' key
	KB_M = 0x32,					//!< 0x32 'M' key
	KB_N = 0x31,					//!< 0x31 'N' key
	KB_O = 0x18,					//!< 0x18 'O' key
	KB_P = 0x19,					//!< 0x19 'P' key
	KB_Q = 0x10,					//!< 0x10 'Q' key
	KB_R = 0x13,					//!< 0x13 'R' key
	KB_S = 0x1F,					//!< 0x1F 'S' key
	KB_T = 0x14,					//!< 0x14 'T' key
	KB_U = 0x16,					//!< 0x16 'U' key
	KB_V = 0x2F,					//!< 0x2F 'V' key
	KB_W = 0x11,					//!< 0x11 'W' key
	KB_X = 0x2D,					//!< 0x2D 'X' key
	KB_Y = 0x15,					//!< 0x15 'Y' key
	KB_Z = 0x2C,					//!< 0x2C 'Z' key

									// Special Keys

	KB_Alt_Left = 0x38,				//!< 0x38 Left Alt
	KB_Alt_Right = 0xB8,			//!< 0xB8 Right Alt  
	KB_Backspace = 0x0E,			//!< 0x0E Backspace
	KB_CapsLock = 0x3A,				//!< 0x3A Capslock
	KB_Comma = 0x33,				//!< 0x33 Comma
	KB_Control_Left = 0x1D,			//!< 0x1D Left Control
	KB_Control_Right = 0x9D,		//!< 0x9D Right Control
	KB_Delete = 0xD3,				//!< 0xD3 Delete
	KB_Forward_Slash = 0x35,				//!< 0x35 Divide
	KB_Full_Stop = 0x34,			//!< 0x34 Fullstop / Greater Than
	KB_End = 0xCF,					//!< 0xCF End
	KB_Enter = 0x1C,				//!< 0x1C Enter
	KB_Equals = 0x0D,				//!< 0x0D Equals
	KB_Home = 0xC7,					//!< 0xC7 Home
	KB_Insert = 0xD2,				//!< 0xD2 Insert
	KB_Minus = 0x0C,				//!< 0x0C Minus / Dash 
	KB_Numlock = 0x45,				//!< 0x45 Numlock
	KB_Page_Down = 0xD1,			//!< 0xD1 Page Down
	KB_Page_Up = 0xC9,				//!< 0xC9 Page Up
	KB_Pause = 0xC5,				//!< 0xC5 Pause
	KB_Print_Screen = 0xB7,			//!< 0xB7 Print_Screen
	KB_Secondary_Action = 0xDD,		//!< 0XDD Secondary Action (not on all keyboards)
	KB_Semicolon = 0x27,			//!< 0x27 Semicolon / Less Than
	KB_ScrollLock = 0x46,			//!< 0x46 Scrolllock
	KB_Shift_Left = 0x2A,			//!< 0x2A Left Shift
	KB_Shift_Right = 0x36,			//!< 0x36 Right Shift
	KB_Single_Quote = 0x28,			//!< 0x28 Single Quote / Double Quote
	KB_Space_Bar = 0x39,				//!< 0x39 Spacebar
	KB_Square_Bracket_Open = 0x1A,	//!< 0x1A Open Square Bracket 
	KB_Square_Bracket_Close = 0x1B, //!< 0x1B Closed Square Bracket
	KB_Tab = 0x0F,					//!< 0x0F Tab
	KB_Tilde = 0x29,				//!< 0x29 Tilde
	KB_Back_Slash = 0x2B,				//!< 0x2B BackSlash (Used by VX1 to open menu)

									// Function Keys

	KB_F1 = 0x3B,					//!< 0x3B F1 function key
	KB_F2 = 0x3C,					//!< 0x3C F2 function key
	KB_F3 = 0x3D,					//!< 0x3D F3 function key
	KB_F4 = 0x3E,					//!< 0x3E F4 function key
	KB_F5 = 0x3F,					//!< 0x3F F5 function key
	KB_F6 = 0x40,					//!< 0x40 F6 function key
	KB_F7 = 0x41,					//!< 0x41 F7 function key
	KB_F8 = 0x42,					//!< 0x42 F8 function key
	KB_F9 = 0x43,					//!< 0x43 F9 function key
	KB_F10 = 0x44,					//!< 0x44 F10 function key
	KB_F11 = 0x57,					//!< 0x57 F11 function key
	KB_F12 = 0x58,					//!< 0x58 F12 function key

									// Numpad

	KB_NUMPAD_0 = 0x52,				//!< 0x52 Numpad 0 key
	KB_NUMPAD_1 = 0x4F,				//!< 0x4F Numpad 1 key
	KB_NUMPAD_2 = 0x50,				//!< 0x50 Numpad 2 key	
	KB_NUMPAD_3 = 0x51,				//!< 0x51 Numpad 3 key
	KB_NUMPAD_4 = 0x4B,				//!< 0x4B Numpad 4 key
	KB_NUMPAD_5 = 0x4C,				//!< 0x4C Numpad 5 key
	KB_NUMPAD_6 = 0x4D,				//!< 0x4D Numpad 6 key
	KB_NUMPAD_7 = 0x47,				//!< 0x47 Numpad 7 key
	KB_NUMPAD_8 = 0x48,				//!< 0x48 Numpad 8 key
	KB_NUMPAD_9 = 0x49,				//!< 0x49 Numpad 9 key
	KB_NUMPAD_Decimal = 0x53,		//!< 0x53 Numpad decimal key
	KB_NUMPAD_Divide = 0xB5,		//!< 0xB5 Numpad divide key
	KB_NUMPAD_Multiply = 0x37,		//!< 0x37 Numpad multiply key
	KB_NUMPAD_Minus = 0x4A,			//!< 0x4A Numpad minus key
	KB_NUMPAD_Plus = 0x4E,			//!< 0x4E Numpad plus key
	KB_NUMPAD_Enter = 0x9C,			//!< 0x9C Numpad enter key

									// Arrow Keys

	KB_Arrow_Right = 0xCD,			//!< 0xCD Arrow Right
	KB_Arrow_Left = 0xCB,			//!< 0xCB Arrow Left
	KB_Arrow_Up = 0xC8,				//!< 0xC8 Arrow Up
	KB_Arrow_Down = 0xD0,			//!< 0xD0 Arrow Down
};


/**
* These are the key scancodes that voxiebox.dll uses for touch controls input.
* if they don't have the extra values the voxiebox.dll doesn't seem to detect the keys properly. 
*/
enum Keycodes_Extended
{
	EKB_ = 0x00,						//!< 0x00 Empty state 
	EKB_Escape = 0x011b,				//!< 0x01 'Esc' / Escape ley

									// Numeric Keys

	EKB_1 = 0x0231,					//!< 0x02 Number 1 key
	EKB_2 = 0x0332,					//!< 0x03 Number 2 key
	EKB_3 = 0x0433,					//!< 0x04 Number 3 key
	EKB_4 = 0x0534,					//!< 0x05 Number 4 key
	EKB_5 = 0x0635,					//!< 0x06 Number 5 key
	EKB_6 = 0x0736,					//!< 0x07 Number 6 key
	EKB_7 = 0x0837,					//!< 0x08 Number 7 key
	EKB_8 = 0x0938,					//!< 0x09 Number 8 key
	EKB_9 = 0x0a39,					//!< 0x0A Number 9 key
	EKB_0 = 0x0b30,					//!< 0x0B Number 0 key

									// Alpha Keys

	EKB_A = 0x1e61,					//!< 0x1E 'A' key
	EKB_B = 0x3000,					//!< 0x30 'B' key
	EKB_C = 0x2E00,					//!< 0x2E 'C' key
	EKB_D = 0x2074,					//!< 0x20 'D' key
	EKB_E = 0x1265,					//!< 0x12 'E' key
	EKB_F = 0x2166,					//!< 0x21 'F' key
	EKB_G = 0x2267,					//!< 0x22 'G' key
	EKB_H = 0x2368,					//!< 0x23 'H' key
	EKB_I = 0x1769,					//!< 0x17 'I' key
	EKB_J = 0x246a,					//!< 0x24 'J' key
	EKB_K = 0x256b,					//!< 0x25 'K' key
	EKB_L = 0x266c,					//!< 0x26 'L' key
	EKB_M = 0x326d,					//!< 0x32 'M' key
	EKB_N = 0x316e,					//!< 0x31 'N' key
	EKB_O = 0x186f,					//!< 0x18 'O' key
	EKB_P = 0x1970,					//!< 0x19 'P' key
	EKB_Q = 0x1071,					//!< 0x10 'Q' key
	EKB_R = 0x1372,					//!< 0x13 'R' key
	EKB_S = 0x1f73,					//!< 0x1F 'S' key
	EKB_T = 0x1474,					//!< 0x14 'T' key
	EKB_U = 0x1675,					//!< 0x16 'U' key
	EKB_V = 0x2F76,					//!< 0x2F 'V' key
	EKB_W = 0x1177,					//!< 0x11 'W' key
	EKB_X = 0x2D78,					//!< 0x2D 'X' key
	EKB_Y = 0x1579,					//!< 0x15 'Y' key
	EKB_Z = 0x2C7a,					//!< 0x2C 'Z' key

									// Special Keys

	EKB_Alt_Left = 0x3800,			//!< 0x38 Left Alt
	EKB_Alt_Right = 0xb800,			//!< 0xB8 Right Alt  
	EKB_Backspace = 0x0e08,			//!< 0x0E Backspace
	EKB_CapsLock = 0x3a00,			//!< 0x3A Capslock
	EKB_Comma = 0x332c,				//!< 0x33 Comma
	EKB_Control_Left = 0x1d00,		//!< 0x1D Left Control
	EKB_Control_Right = 0x9d00,		//!< 0x9D Right Control
	EKB_Delete = 0xd300,			//!< 0xD3 Delete
	EKB_Forward_Slash = 0x352f,		//!< 0x35 Forward Slash (same as divide)
	EKB_Full_Stop = 0x342e,			//!< 0x34 Fullstop / Greater Than
	EKB_End = 0xcf00,				//!< 0xCF End
	EKB_Enter = 0x1c0d,				//!< 0x1C Enter
	EKB_Equals = 0x0d3d,			//!< 0x0D Equals
	EKB_Home = 0xc700,				//!< 0xC7 Home
	EKB_Insert = 0xd200,			//!< 0xD2 Insert
	EKB_Minus = 0x0c2d,				//!< 0x0C Minus / Dash 
	EKB_Numlock = 0x4500,			//!< 0x45 Numlock
	EKB_Page_Down = 0xd100,			//!< 0xD1 Page Down
	EKB_Page_Up = 0xc900,			//!< 0xC9 Page Up
	EKB_Pause_Break = 0xC500,			//!< 0xC5 Pause
	EKB_Print_Screen = 0xB700,			//!< 0xB7 Print_Screen
	EKB_Secondary_Action = 0xDD00,		//!< 0XDD Secondary Action (not on all keyboards)
	EKB_Semicolon = 0x273b,				//!< 0x27 Semicolon / Less Than
	EKB_ScrollLock = 0x4600,			//!< 0x46 Scrolllock
	EKB_Shift_Left = 0x2a00,			//!< 0x2A Left Shift
	EKB_Shift_Right = 0x3600,			//!< 0x36 Right Shift
	EKB_Single_Quote = 0x2827,			//!< 0x28 Single Quote / Double Quote
	EKB_Space_Bar = 0x3920,				//!< 0x39 Spacebar
	EKB_Square_Bracket_Open = 0x1a5b,	//!< 0x1A Open Square Bracket 
	EKB_Square_Bracket_Close = 0x1b5d, 	//!< 0x1B Closed Square Bracket
	EKB_Tab = 0x0f09,					//!< 0x0F Tab
	EKB_Tilde = 0x2960,					//!< 0x29 Tilde
	EKB_Back_Slash = 0x2b5c,			//!< 0x2B BackSlash (Used by VX1 to open menu)

									// Function Keys

	EKB_F1 = 0x3b00,					//!< 0x3B F1 function key
	EKB_F2 = 0x3C00,					//!< 0x3C F2 function key
	EKB_F3 = 0x3D00,					//!< 0x3D F3 function key
	EKB_F4 = 0x3E00,					//!< 0x3E F4 function key
	EKB_F5 = 0x3F00,					//!< 0x3F F5 function key
	EKB_F6 = 0x4000,					//!< 0x40 F6 function key
	EKB_F7 = 0x4100,					//!< 0x41 F7 function key
	EKB_F8 = 0x4200,					//!< 0x42 F8 function key
	EKB_F9 = 0x4300,					//!< 0x43 F9 function key
	EKB_F10 = 0x4400,					//!< 0x44 F10 function key
	EKB_F11 = 0x5700,					//!< 0x57 F11 function key
	EKB_F12 = 0x5800,					//!< 0x58 F12 function key

									// Numpad

	EKB_NUMPAD_0 = 0x5230,				//!< 0x52 Numpad 0 key
	EKB_NUMPAD_1 = 0x4F31,				//!< 0x4F Numpad 1 key
	EKB_NUMPAD_2 = 0x5032,				//!< 0x50 Numpad 2 key	
	EKB_NUMPAD_3 = 0x5133,				//!< 0x51 Numpad 3 key
	EKB_NUMPAD_4 = 0x4B34,				//!< 0x4B Numpad 4 key
	EKB_NUMPAD_5 = 0x4C35,				//!< 0x4C Numpad 5 key
	EKB_NUMPAD_6 = 0x4D36,				//!< 0x4D Numpad 6 key
	EKB_NUMPAD_7 = 0x4737,				//!< 0x47 Numpad 7 key
	EKB_NUMPAD_8 = 0x4838,				//!< 0x48 Numpad 8 key
	EKB_NUMPAD_9 = 0x4939,				//!< 0x49 Numpad 9 key
	EKB_NUMPAD_Decimal = 0x532e,		//!< 0x53 Numpad decimal key
	EKB_NUMPAD_Divide = 0xB500,			//!< 0xB5 Numpad divide key
	EKB_NUMPAD_Multiply = 0x372a,		//!< 0x37 Numpad multiply key
	EKB_NUMPAD_Minus = 0x4A2d,			//!< 0x4A Numpad minus key
	EKB_NUMPAD_Plus = 0x4E2b,			//!< 0x4E Numpad plus key
	EKB_NUMPAD_Enter = 0x9c00,			//!< 0x9C Numpad enter key

									// Arrow Keys

	EKB_Arrow_Right = 0xcd00,			//!< 0xCD Arrow Right    
	EKB_Arrow_Left = 0xcb00,			//!< 0xCB Arrow Left
	EKB_Arrow_Up = 0xC800,				//!< 0xC8 Arrow Up
	EKB_Arrow_Down = 0xd000,			//!< 0xD0 Arrow Down
};



/*
 To make a custom touch keyboard layout you need to define an array of const touchkey_t 
 each touchkey_t struct contains one key the params are

	char* 	title		- the title of the key
	int 	xpos  		- the x position of where to draw the touch button 
	int 	ypos  		- the y position of where to draw the touch button 
	int 	xsize 		- the size or the horizontal length of the button written as pixels
	int 	ysize 		- the size or the horizontal length of the button written as pixels
	int		frontColor	- the hexidemical color value for the button
	int		backColor	- the hexidemical color value for the button's background. -1 for transparent
	int 	scanCode	- the scancode / keycode value for the button.

to actually see the custom touch controlls you need to have TOUCH CONTROLLS ENABLED
by default they are turned off - they can be turned on by pressing the button in the 'misc' menu
or adding 'touchcontrols=1' to voxiebox.ini

*/

// COPY OF THE DEFAULT TOUCH KEY FROM VOXIEBOX SOURCE
static const touchkey_t default_touchkey[] =
{
	"`"    ,  0, 80, 64, 64,0x605040,-1,0x2960,
	"1"    , 68, 80, 64, 64,0x605040,-1,0x0231,
	"2"    ,136, 80, 64, 64,0x605040,-1,0x0332,
	"3"    ,204, 80, 64, 64,0x605040,-1,0x0433,
	"4"    ,272, 80, 64, 64,0x605040,-1,0x0534,
	"5"    ,340, 80, 64, 64,0x605040,-1,0x0635,
	"6"    ,408, 80, 64, 64,0x605040,-1,0x0736,
	"7"    ,476, 80, 64, 64,0x605040,-1,0x0837,
	"8"    ,544, 80, 64, 64,0x605040,-1,0x0938,
	"9"    ,612, 80, 64, 64,0x605040,-1,0x0a39,
	"0"    ,680, 80, 64, 64,0x605040,-1,0x0b30,
	"-"    ,748, 80, 64, 64,0x605040,-1,0x0c2d,
	"="    ,816, 80, 64, 64,0x605040,-1,0x0d3d,
	"Backspace",884, 80,139, 64,0x405060,-1,0x0e08,

	"Tab"  ,  0,148, 98, 64,0x605040,-1,0x0f09,
	"Q"    ,102,148, 64, 64,0x605040,-1,0x1071,
	"W"    ,170,148, 64, 64,0x605040,-1,0x1177,
	"E"    ,238,148, 64, 64,0x605040,-1,0x1265,
	"R"    ,306,148, 64, 64,0x605040,-1,0x1372,
	"T"    ,374,148, 64, 64,0x605040,-1,0x1474,
	"Y"    ,442,148, 64, 64,0x605040,-1,0x1579,
	"U"    ,510,148, 64, 64,0x605040,-1,0x1675,
	"I"    ,578,148, 64, 64,0x605040,-1,0x1769,
	"O"    ,646,148, 64, 64,0x605040,-1,0x186f,
	"P"    ,714,148, 64, 64,0x605040,-1,0x1970,
	"["    ,782,148, 64, 64,0x605040,-1,0x1a5b,
	"]"    ,850,148, 64, 64,0x605040,-1,0x1b5d,
	"\\"   ,918,148,105, 64,0x605040,-1,0x2b5c,

	"Caps",0,216, 98, 64,0x405060,-1,0x3a00,
	"A"    ,124,216, 64, 64,0x605040,-1,0x1e61,
	"S"    ,192,216, 64, 64,0x605040,-1,0x1f73,
	"D"    ,260,216, 64, 64,0x605040,-1,0x2074,
	"F"    ,328,216, 64, 64,0x605040,-1,0x2166,
	"G"    ,396,216, 64, 64,0x605040,-1,0x2267,
	"H"    ,464,216, 64, 64,0x605040,-1,0x2368,
	"J"    ,532,216, 64, 64,0x605040,-1,0x246a,
	"K"    ,600,216, 64, 64,0x605040,-1,0x256b,
	"L"    ,668,216, 64, 64,0x605040,-1,0x266c,
	";"    ,736,216, 64, 64,0x605040,-1,0x273b,
	"'"    ,804,216, 64, 64,0x605040,-1,0x2827,
	"Enter",872,216,151, 64,0x605040,-1,0x1c0d,

	"Shift",  0,284,154, 64,0x405060,-1,0x2a00,
	"Z"    ,158,284, 64, 64,0x605040,-1,0x2c7a,
	"X"    ,226,284, 64, 64,0x605040,-1,0x2d78,
	"C"    ,294,284, 64, 64,0x605040,-1,0x2e63,
	"V"    ,362,284, 64, 64,0x605040,-1,0x2f76,
	"B"    ,430,284, 64, 64,0x605040,-1,0x3062,
	"N"    ,498,284, 64, 64,0x605040,-1,0x316e,
	"M"    ,566,284, 64, 64,0x605040,-1,0x326d,
	","    ,634,284, 64, 64,0x605040,-1,0x332c,
	"."    ,702,284, 64, 64,0x605040,-1,0x342e,
	"/"    ,770,284, 64, 64,0x605040,-1,0x352f,
	"Shift",838,284,185, 64,0x405060,-1,0x3600,

	"Ctrl" ,  0,352,100, 64,0x405060,-1,0x1d00,
	"Alt"  ,174,352,100, 64,0x405060,-1,0x3800,
	" "    ,278,352,468, 64,0x605040,-1,0x3920,
	"Alt"  ,750,352,100, 64,0x405060,-1,0xb800,
	"Ctrl" ,924,352, 99, 64,0x405060,-1,0x9d00,

	"LMB"  ,  0,432,116,167,0x406040,-1,0x0001, //LBut
	"Touch Pad",120,432,216,167,0x406040,-1,0x0000, //Mouse
	"RMB"  ,340,432, 96,167,0x406040,-1,0x0002, //RBut
 //"MMB"  ,440,432, 96,167,0x406040,-1,0x0003, //MBut

	"Ins"  ,510,464, 64, 64,0x405060,-1,0xd200,
	"Home" ,578,464, 64, 64,0x405060,-1,0xc700,
	"PgUp" ,646,464, 64, 64,0x405060,-1,0xc900,
	"Del"  ,510,532, 64, 64,0x405060,-1,0xd300,
	"End"  ,578,532, 64, 64,0x405060,-1,0xcf00,
	"PgDn" ,646,532, 64, 64,0x405060,-1,0xd100,

	"ESC"  ,750,420, 64, 64,0x405060,-1,0x011b,

	"\xc8" ,860,432, 80, 80,0x405060,-1,0xc800, // hak to show graphical arrow
	"\xcb" ,776,512, 80, 80,0x405060,-1,0xcb00, // hak to show graphical arrow
	"\xd0" ,860,512, 80, 80,0x405060,-1,0xd000, // hak to show graphical arrow 
	"\xcd" ,944,512, 79, 80,0x405060,-1,0xcd00, // hak to show graphical arrow
};

// COPY OF THE DEFAULT TOUCH KEY FROM VOXIEBOX SOURCE
static const touchkey_t default_touchkey_extended[] =
{

	"ESC"    ,  0, 80, 50, 50,0x405060,-1,EKB_Escape,
	"F1"    , 86, 80, 50, 50,0x405060,-1,EKB_F1,
	"F2"    ,137, 80, 50, 50,0x405060,-1,EKB_F2,
	"F3"    ,188, 80, 50, 50,0x405060,-1,EKB_F3,
	"F4"    ,239, 80, 50, 50,0x405060,-1,EKB_F4,
	"F5"    ,325, 80, 50, 50,0x405060,-1,EKB_F5,
	"F6"    ,376, 80, 50, 50,0x405060,-1,EKB_F6,
	"F7"    ,427, 80, 50, 50,0x405060,-1,EKB_F7,
	"F8"    ,478, 80, 50, 50,0x405060,-1,EKB_F8,
	"F9"    ,564, 80, 50, 50,0x405060,-1,EKB_F9,
	"F10"    ,615, 80, 50, 50,0x405060,-1,EKB_F10,
	"F11"    ,666, 80, 50, 50,0x405060,-1,EKB_F11,
	"F12"    ,717, 80, 50, 50,0x405060,-1,EKB_F12,

	"PrntS" ,841, 80, 60, 50,0x405060,-1,EKB_Print_Screen,
	"Sclk"  ,902, 80, 60, 50,0x405060,-1,EKB_ScrollLock,
	"Pause" ,963, 80, 60, 50,0x405060,-1,EKB_Pause_Break,

	"`"    ,  0, 155, 50, 50,0x605040,-1,EKB_Tilde,
	"1"    , 51, 155, 50, 50,0x605040,-1,EKB_1,
	"2"    ,102, 155, 50, 50,0x605040,-1,EKB_2,
	"3"    ,153, 155, 50, 50,0x605040,-1,EKB_3,
	"4"    ,204, 155, 50, 50,0x605040,-1,EKB_4,
	"5"    ,255, 155, 50, 50,0x605040,-1,EKB_5,
	"6"    ,306, 155, 50, 50,0x605040,-1,EKB_6,
	"7"    ,357, 155, 50, 50,0x605040,-1,EKB_7,
	"8"    ,408, 155, 50, 50,0x605040,-1,EKB_8,
	"9"    ,459, 155, 50, 50,0x605040,-1,EKB_9,
	"0"    ,510, 155, 50, 50,0x605040,-1,EKB_0,
	"-"    ,561, 155, 50, 50,0x605040,-1,EKB_Minus,
	"="    ,612, 155, 50, 50,0x605040,-1,EKB_Equals,
	"Bckspace",663, 155,100, 50,0x405060,-1,EKB_Backspace,

	"Tab"  ,  0,206, 75, 50,0x405060,-1,EKB_Tab,
	"Q"    ,76 , 206, 50, 50,0x605040,-1,EKB_Q,
	"W"    ,127,206, 50, 50,0x605040,-1,EKB_W,
	"E"    ,178,206, 50, 50,0x605040,-1,EKB_E,
	"R"    ,229,206, 50, 50,0x605040,-1,EKB_R,
	"T"    ,280,206, 50, 50,0x605040,-1,EKB_T,
	"Y"    ,331,206, 50, 50,0x605040,-1,EKB_Y,
	"U"    ,382,206, 50, 50,0x605040,-1,EKB_U,
	"I"    ,433,206, 50, 50,0x605040,-1,EKB_I,
	"O"    ,484,206, 50, 50,0x605040,-1,EKB_O,
	"P"    ,535,206, 50, 50,0x605040,-1,EKB_P,
	"["    ,586,206, 50, 50,0x605040,-1,EKB_Square_Bracket_Open,
	"]"    ,637,206, 50, 50,0x605040,-1,EKB_Square_Bracket_Close,
	"\\"   ,688,206, 75, 50,0x605040,-1,EKB_Back_Slash,

	"Caps"	,0 ,257, 75, 50,0x405060,-1,EKB_CapsLock,
	"A"    ,76 ,257, 50, 50,0x605040,-1,EKB_A,
	"S"    ,127,257, 50, 50,0x605040,-1,EKB_B,
	"D"    ,178,257, 50, 50,0x605040,-1,EKB_D,
	"F"    ,229,257, 50, 50,0x605040,-1,EKB_F,
	"G"    ,280,257, 50, 50,0x605040,-1,EKB_G,
	"H"    ,331,257, 50, 50,0x605040,-1,EKB_H,
	"J"    ,382,257, 50, 50,0x605040,-1,EKB_J,
	"K"    ,433,257, 50, 50,0x605040,-1,EKB_K,
	"L"    ,484,257, 50, 50,0x605040,-1,EKB_L,
	";"    ,535,257, 50, 50,0x605040,-1,EKB_Semicolon,
	"'"    ,586,257, 50, 50,0x605040,-1,EKB_Single_Quote,
	"Enter",637,257,125, 50,0x605040,-1,EKB_Enter,

	"Shift",  0,308,100,  50,0x405060,-1,EKB_Shift_Left,
	"Z"    ,101,308, 50, 50,0x605040,-1,EKB_Z,
	"X"    ,152,308, 50, 50,0x605040,-1,EKB_X,
	"C"    ,203,308, 50, 50,0x605040,-1,EKB_C,
	"V"    ,254,308, 50, 50,0x605040,-1,EKB_V,
	"B"    ,305,308, 50, 50,0x605040,-1,EKB_B,
	"N"    ,356,308, 50, 50,0x605040,-1,EKB_N,
	"M"    ,407,308, 50, 50,0x605040,-1,EKB_M,
	","    ,458,308, 50, 50,0x605040,-1,EKB_Comma,
	"."    ,509,308, 50, 50,0x605040,-1,EKB_Full_Stop,
	"/"    ,560,308, 50, 50,0x605040,-1,EKB_Forward_Slash,
	"Shift",611,308,150,  50,0x405060,-1,EKB_Shift_Right,

	"Ctrl" ,  0,359,75, 50,0x405060,-1,EKB_Control_Left,
	"Alt"  ,76,359,75, 50,0x405060,-1,EKB_Alt_Left,
	"Space"    ,152,359,460, 50,0x605040,-1,EKB_Space_Bar,
	"Alt"  ,613,359,75, 50,0x405060,-1,EKB_Alt_Right,
	"Ctrl" ,689,359, 75, 50,0x405060,-1,EKB_Control_Right,

	"LMB"  ,  0,432,116,167,0x406040,-1,0x0001, //LBut
	"Touch Pad",120,432,216,167,0x406040,-1,0x0000, //Mouse
	"RMB"  ,340,432, 96,167,0x406040,-1,0x0002, //RBut
 //"MMB"  ,440,432, 96,167,0x406040,-1,0x0003, //MBut

	"Ins"  ,490,493, 60, 50,0x405060,-1,EKB_Insert,
	"Home" ,551,493, 60, 50,0x405060,-1,EKB_Home,
	"PgUp" ,612,493, 60, 50,0x405060,-1,EKB_Page_Up,

	"Del"  ,490,544, 60, 50,0x405060,-1,EKB_Delete,
	"End"  ,551,544, 60, 50,0x405060,-1,EKB_End,
	"PgDn" ,612,544, 60, 50,0x405060,-1,EKB_Page_Down,

	"\xc8" ,860,432, 80, 80,0x405060,-1,EKB_Arrow_Up, // hak to show graphical arrow
	"\xcb" ,776,512, 80, 80,0x405060,-1,EKB_Arrow_Left, // hak to show graphical arrow
	"\xd0" ,860,512, 80, 80,0x405060,-1,EKB_Arrow_Right, // hak to show graphical arrow 
	"\xcd" ,944,512, 79, 80,0x405060,-1,EKB_Arrow_Down, // hak to show graphical arrow


	"Num"  ,  819, 155, 50, 50,0x405060,-1,EKB_Numlock,
	"/"    ,  870, 155, 50, 50,0x405060,-1,EKB_NUMPAD_Divide,
	"*"    ,  922, 155, 50, 50,0x405060,-1,EKB_NUMPAD_Multiply,
	"-"    ,  973, 155, 50, 50,0x405060,-1,EKB_NUMPAD_Minus,

	"7"    ,  819, 206, 50, 50,0x605040,-1,EKB_NUMPAD_7,
	"8"    ,  870, 206, 50, 50,0x605040,-1,EKB_NUMPAD_8,
	"9"    ,  922, 206, 50, 50,0x605040,-1,EKB_NUMPAD_9,
	"+"    ,  973, 206, 50, 100,0x405060,-1,EKB_NUMPAD_Plus,

	"4"    ,  819, 257, 50, 50,0x605040,-1,EKB_NUMPAD_4,
	"5"    ,  870, 257, 50, 50,0x605040,-1,EKB_NUMPAD_5,
	"6"    ,  922, 257, 50, 50,0x605040,-1,EKB_NUMPAD_6,

	"1"    ,  819, 308, 50, 50,0x605040,-1,EKB_NUMPAD_1,
	"2"    ,  870, 308, 50, 50,0x605040,-1,EKB_NUMPAD_2,
	"3"    ,  922, 308, 50, 50,0x605040,-1,EKB_NUMPAD_3,
	"Ent"    ,  973, 308, 50, 100,0x405060,-1,EKB_NUMPAD_Enter,

	"0"    ,  819, 359, 100, 50,0x605040,-1,EKB_NUMPAD_0,
	"."    ,  922, 359, 50, 50,0x605040,-1,EKB_NUMPAD_Decimal,

};

static const touchkey_t bigArrows_touchkey[] =
{
	"\xc8" ,600, 100, 200, 200,0x800000,0x100000,EKB_Arrow_Up,
	"\xcb" ,390, 310, 200, 200,0x800000,0x100000,EKB_Arrow_Left,
	"\xd0" ,600, 310, 200, 200,0x800000,0x100000,EKB_Arrow_Down,
	"\xcd" ,810, 310, 200, 200,0x800000,0x100000,EKB_Arrow_Right,

	"A"    ,75, 100, 100, 100,0x008000,0x001000,EKB_A,
	"Z"    ,75, 210, 100, 100,0x008000,0x001000,EKB_Z,
	"S"    ,185, 100, 100, 100,0x008000,0x001000,EKB_S,
	"X"    ,185, 210, 100, 100,0x008000,0x001000,EKB_X,

	"Ctrl"    ,25, 400, 150, 100,0x008080,0x001010,EKB_Control_Right,
	"Shift"    ,200, 400, 150, 100,0x008080,0x001010,EKB_Shift_Right,

};

static const touchkey_t bigMouse_touchkey[] =
{
	"Touch Pad",2,70,1020,415,0x406040,0x051005,0x0000, //Mouse
	"LMB"  ,  2,490,340,110,0x406040,0x051005,0x0001, //LBut
 //	"MMB"  ,344,490,340,110,0x406040,0x051005,0x0003, //MBut
	"RMB"  ,686,490,340,110,0x406040,0x051005,0x0002, //RBut

};

int WINAPI WinMain(HINSTANCE hinst, HINSTANCE hpinst, LPSTR cmdline, int ncmdshow)
{
	voxie_inputs_t in; 
	pol_t pt[3];
	double tim = 0.0, otim, dtim, avgdtim = 0.0;
	int i;
	int inited = 0; 
	static int touchKeyArrayNum; 
	char * message = " ";


	if (voxie_load(&vw) < 0) //Load voxiebox.dll and get settings from voxiebox.ini. May override settings in vw structure here if desired.
	{
		MessageBox(0, "Error: can't load voxiebox.dll", "", MB_OK); // if there is an error this will create a standard Windows message box
		return (-1);
	}
	if (voxie_init(&vw) < 0) //Start video and audio.
	{						
		return (-1);
	}

	

	while (!voxie_breath(&in)) // a breath is a complete volume sweep. a whole volume is rendered in a single breath
	{
		otim = tim; 
		tim = voxie_klock(); 
		dtim = tim - otim; 

		if (voxie_keystat(0x1)) // esc key closes app
		{
			voxie_quitloop(); // quitloop() is used to exit the main loop of the program
		}
		i = (voxie_keystat(0x1b) & 1) - (voxie_keystat(0x1a) & 1); // keys '[' and ']'
		if (i)
		{
			if (voxie_keystat(0x2a) | voxie_keystat(0x36))
				vw.emuvang = min(max(vw.emuvang + (float)i * dtim * 2.0, -PI * .5), 0.1268); //Shift+[,]
			else if (voxie_keystat(0x1d) | voxie_keystat(0x9d))
				vw.emudist = max(vw.emudist - (float)i * dtim * 2048.0, 400.0); //Ctrl+[,]
			else
				vw.emuhang += (float)i * dtim * 2.0; //[,]
			voxie_init(&vw);
		}

		if (voxie_keystat(KB_1)) // 1 key loads default touch key setup
		{ 
			touchKeyArrayNum = sizeof(default_touchkey)/sizeof(default_touchkey[0]);
			voxie_touch_custom(default_touchkey,touchKeyArrayNum);
			message = "Default Keyboard loaded";
		}	
 

		if (voxie_keystat(KB_2)) // 2 key loads big arrows touch key setup
		{ 
			touchKeyArrayNum = sizeof(bigArrows_touchkey)/sizeof(bigArrows_touchkey[0]);
			voxie_touch_custom(bigArrows_touchkey,touchKeyArrayNum);
			message = "Big Arrows loaded";
		}	

		if (voxie_keystat(KB_3)) // 3 key loads extended touch key setup 
		{ 
			touchKeyArrayNum = sizeof(default_touchkey_extended)/sizeof(default_touchkey_extended[0]);
			voxie_touch_custom(default_touchkey_extended,touchKeyArrayNum);
			message = "Default Keyboard Extended";
		}	

		if (voxie_keystat(KB_4)) // 4 key loads big mouse touch key setup
		{ 
			touchKeyArrayNum = sizeof(bigMouse_touchkey)/sizeof(default_touchkey_extended[0]);
			voxie_touch_custom(bigMouse_touchkey,touchKeyArrayNum);
			message = "Big Mouse";
		}	



		// DRAW **************************

		voxie_frame_start(&vf); 
		voxie_setview(&vf, -vw.aspx, -vw.aspy, -vw.aspz, vw.aspx, vw.aspy, vw.aspz); 



		voxie_debug_print6x8_(100,60,0x00ff00,0x000000,"To enable touch keyboard select 'Touch Controls' under the Misc. tab"); // types out a message
		voxie_debug_print6x8_(550,60,0xffff00,0x000000,"%s", message); // types out a message
			
#if DEBUG	
		//draw wireframe box around the edge of the screen
		voxie_drawbox(&vf, -vw.aspx + 1e-3, -vw.aspy + 1e-3, -vw.aspz, +vw.aspx - 1e-3, +vw.aspy - 1e-3, +vw.aspz, 1, 0xffffff);

		//display VPS
		avgdtim += (dtim - avgdtim) * .1;
		voxie_debug_print6x8_(30, 60, 0xffc080, -1, "VPS %5.1f", 1.0 / avgdtim); 	
#endif

	voxie_frame_end(); 
	
	voxie_getvw(&vw);

	}


	voxie_uninit(0); //Close window and unload voxiebox.dll
	return (0);
}

#if 0
!endif
#endif

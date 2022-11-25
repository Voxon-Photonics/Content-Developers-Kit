#pragma once

/** \file vxInputTypes.h
 *
 * VX++ input types header contains definitions to do with various input systems.
 * This header is essential to developing VX++ applications using the VxCpp.DLL.
 * This file is kept separate to allow easy integration...
 *
 */

#ifndef VX_INPUT_TYPES
#define VX_INPUT_TYPES

//! \int constant to define the max amount of USB game controllers. 
const int MAX_CONTROLLERS = 4;

//! \enum Gamepad / Joystick button codes
/*! Joystick buttoncodes inputs for standard USB game controllers. Xinput standard
*	the Voxiebox.dll automatically converts these inputs to DirectInput 
*   if the voxie_wind_t controls weather the Voxiebox.dll is to read Xinput or DirectInput APIs 
*   (@see voxieBox::setJoyInputToXInput @see voxieBox::setJoyInputToDirectInput)
*	All these controls are digital (1 = On / 0 = Off) for Analog controls @see ControllerAxis @see ControllerTrigger 
*	To read gamepad input @see voxie_xbox_t @see voxieBox::updateJoyState
*/
enum JoyButtonCodes
{
 	JOY_DPAD_UP				= 0,	//!< bit 0 value 1			Digital Dpad Up				
 	JOY_DPAD_DOWN			= 1,	//!< bit 1 value 2			Digital Dpad Down 
 	JOY_DPAD_LEFT			= 2,	//!< bit 2 value 4			Digital Dpad Left
 	JOY_DPAD_RIGHT			= 3,	//!< bit 3 value 8			Digital Dpad Right
 	JOY_START				= 4,	//!< bit 4 value 16			Start Button
 	JOY_BACK				= 5,	//!< bit 5 value 32			Back Button
 	JOY_LEFT_THUMB			= 6,	//!< bit 6 value 64			Left Thumb Stick Button (when you press 'down' on the left analog stick)
 	JOY_RIGHT_THUMB			= 7,	//!< bit 7 value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
 	JOY_LEFT_SHOULDER		= 8,	//!< bit 8 value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
 	JOY_RIGHT_SHOULDER		= 9,	//!< bit 9 value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
 	JOY_A					= 12,	//!< bit 12 value 1,024		The 'A' Button on a standard Xbox Controller
 	JOY_B					= 13,	//!< bit 13 value 2,048		The 'B' Button on a standard Xbox Controller
 	JOY_X					= 14,	//!< bit 14 value 4,096		The 'X' Button on a standard Xbox Controller
 	JOY_Y					= 15	//!< bit 15 value 8,192		The 'Y' Button on a standard Xbox Controller

};
//! \enum Controller Axis Enum helps identify which analog input is being used 
enum ControllerAxis { 
	JOY_LEFT_STICK_X,			//!< 0 Analog Left Stick X axis
	JOY_LEFT_STICK_Y,			//!< 1 Analog Left Stick Y axis
	JOY_RIGHT_STICK_X,			//!< 2 Analog Right Stick X axis
	JOY_RIGHT_STICK_Y,			//!< 3 Analog Right Stick Y axis
};

//! \enum Controller Trigger. These are the two shoulder analog triggers
enum ControllerTrigger {
	JOY_LEFT_TRIGGER,		//!< 0 Analog Left Shoulder trigger 
	JOY_RIGHT_TRIGGER,		//!< 1 Analog Right Shoulder trigger
};

//! \enum the input codes for mouse and SpaceNav buttons 
enum MouseButton {
	MOUSE_LEFT_BUTTON ,			//!< bit 0 value 1			Left Mouse Button
	MOUSE_RIGHT_BUTTON ,		//!< bit 1 value 2			Right Mouse Button
	MOUSE_MIDDLE_BUTTON			//!< bit 2 value 4			Middle Mouse Button
};

//! \enum the input codes for mouse and SpaceNav buttons 
enum NavButton {
	NAV_LEFT_BUTTON,			//!< bit 0 value 1 Left  SpaceNav Button
	NAV_RIGHT_BUTTON,			//!< bit 1 value 2 Right SpaceNav Button
};

enum NavAxis {
	NAV_X_AXIS,			//!< 0 Left SpaceNav Button
	NAV_Y_AXIS,			//!< 1 RightSpaceNav Button
	NAV_Z_AXIS
};


//! \enum Keyboard scancodes. 
/**
* These are the scancodes that voxieBox::getKeyState, voxieBox::getKeyIsDown, 
* voxieBox::getKeyOnDown, voxieBox:getKeyStreamScanCode used to detect key presses.
*/
enum Keycodes
{
	KB_ = 0x00,						//!< 0x00 Empty state 
	KB_Escape = 0x01,				//!< 0x01 'Esc' / Escape key

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
	KB_Forward_Slash = 0x35,		//!< 0x35 Divide
	KB_Full_Stop = 0x34,			//!< 0x34 Fullstop / Greater Than
	KB_End = 0xCF,					//!< 0xCF End
	KB_Enter = 0x1C,				//!< 0x1C Enter
	KB_Equals = 0x0D,				//!< 0x0D Equals
	KB_Home = 0xC7,					//!< 0xC7 Home
	KB_Insert = 0xD2,				//!< 0xD2 Insert
	KB_Minus = 0x0C,				//!< 0x0C Minus / Dash 
	KB_Numlock = 0xC5,				//!< 0xC5 Numlock
	KB_Page_Down = 0xD1,			//!< 0xD1 Page Down
	KB_Page_Up = 0xC9,				//!< 0xC9 Page Up
	KB_Pause = 0x45,				//!< 0x45 Pause
	KB_Print_Screen = 0xB7,			//!< 0xB7 Print_Screen
	KB_Secondary_Action = 0xDD,		//!< 0XDD Secondary Action (not on all keyboards)
	KB_Semicolon = 0x27,			//!< 0x27 Semicolon / Less Than
	KB_ScrollLock = 0x46,			//!< 0x46 Scrolllock
	KB_Shift_Left = 0x2A,			//!< 0x2A Left Shift
	KB_Shift_Right = 0x36,			//!< 0x36 Right Shift
	KB_Single_Quote = 0x28,			//!< 0x28 Single Quote / Double Quote
	KB_Space_Bar = 0x39,			//!< 0x39 Spacebar
	KB_Square_Bracket_Open = 0x1A,	//!< 0x1A Open Square Bracket 
	KB_Square_Bracket_Close = 0x1B, //!< 0x1B Closed Square Bracket
	KB_Tab = 0x0F,					//!< 0x0F Tab
	KB_Tilde = 0x29,				//!< 0x29 Tilde
	KB_Back_Slash = 0x2B,			//!< 0x2B BackSlash (Used by VX1 to open menu)

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



//! \enum Keyboard scancodes touch layout..  
/**
* These are scancodes with an ASCII code amended to the end. touchkey_t needs both values to work properly. @see input_types.h::touchkey_t
*/

enum Keycodes_Touch
{
	TKB_ = 0x00,						//!< 0x00 Empty state 
	TKB_Escape = 0x011b,				//!< 0x01 'Esc' / Escape key

// Numeric Keys

TKB_1 = 0x0231,					//!< 0x02 Number 1 key
TKB_2 = 0x0332,					//!< 0x03 Number 2 key
TKB_3 = 0x0433,					//!< 0x04 Number 3 key
TKB_4 = 0x0534,					//!< 0x05 Number 4 key
TKB_5 = 0x0635,					//!< 0x06 Number 5 key
TKB_6 = 0x0736,					//!< 0x07 Number 6 key
TKB_7 = 0x0837,					//!< 0x08 Number 7 key
TKB_8 = 0x0938,					//!< 0x09 Number 8 key
TKB_9 = 0x0a39,					//!< 0x0A Number 9 key
TKB_0 = 0x0b30,					//!< 0x0B Number 0 key

// Alpha Keys

TKB_A = 0x1e61,					//!< 0x1E 'A' key
TKB_B = 0x3000,					//!< 0x30 'B' key
TKB_C = 0x2E00,					//!< 0x2E 'C' key
TKB_D = 0x2074,					//!< 0x20 'D' key
TKB_E = 0x1265,					//!< 0x12 'E' key
TKB_F = 0x2166,					//!< 0x21 'F' key
TKB_G = 0x2267,					//!< 0x22 'G' key
TKB_H = 0x2368,					//!< 0x23 'H' key
TKB_I = 0x1769,					//!< 0x17 'I' key
TKB_J = 0x246a,					//!< 0x24 'J' key
TKB_K = 0x256b,					//!< 0x25 'K' key
TKB_L = 0x266c,					//!< 0x26 'L' key
TKB_M = 0x326d,					//!< 0x32 'M' key
TKB_N = 0x316e,					//!< 0x31 'N' key
TKB_O = 0x186f,					//!< 0x18 'O' key
TKB_P = 0x1970,					//!< 0x19 'P' key
TKB_Q = 0x1071,					//!< 0x10 'Q' key
TKB_R = 0x1372,					//!< 0x13 'R' key
TKB_S = 0x1f73,					//!< 0x1F 'S' key
TKB_T = 0x1474,					//!< 0x14 'T' key
TKB_U = 0x1675,					//!< 0x16 'U' key
TKB_V = 0x2F76,					//!< 0x2F 'V' key
TKB_W = 0x1177,					//!< 0x11 'W' key
TKB_X = 0x2D78,					//!< 0x2D 'X' key
TKB_Y = 0x1579,					//!< 0x15 'Y' key
TKB_Z = 0x2C7a,					//!< 0x2C 'Z' key

// Special Keys

TKB_Alt_Left = 0x3800,			//!< 0x38 Left Alt
TKB_Alt_Right = 0xb800,			//!< 0xB8 Right Alt  
TKB_Backspace = 0x0e08,			//!< 0x0E Backspace
TKB_CapsLock = 0x3a00,			//!< 0x3A Capslock
TKB_Comma = 0x332c,				//!< 0x33 Comma
TKB_Control_Left = 0x1d00,		//!< 0x1D Left Control
TKB_Control_Right = 0x9d00,		//!< 0x9D Right Control
TKB_Delete = 0xd300,			//!< 0xD3 Delete
TKB_Forward_Slash = 0x352f,		//!< 0x35 Forward Slash (same as divide)
TKB_Full_Stop = 0x342e,			//!< 0x34 Fullstop / Greater Than
TKB_End = 0xcf00,				//!< 0xCF End
TKB_Enter = 0x1c0d,				//!< 0x1C Enter
TKB_Equals = 0x0d3d,			//!< 0x0D Equals
TKB_Home = 0xc700,				//!< 0xC7 Home
TKB_Insert = 0xd200,			//!< 0xD2 Insert
TKB_Minus = 0x0c2d,				//!< 0x0C Minus / Dash 
TKB_Numlock = 0xC500,			//!< 0xC500 Numlock
TKB_Page_Down = 0xd100,			//!< 0xD1 Page Down
TKB_Page_Up = 0xc900,			//!< 0xC9 Page Up
TKB_Pause_Break = 0x4500,			//!< 0x4500 Pause
TKB_Print_Screen = 0xB700,			//!< 0xB7 Print_Screen
TKB_Secondary_Action = 0xDD00,		//!< 0XDD Secondary Action (not on all keyboards)
TKB_Semicolon = 0x273b,				//!< 0x27 Semicolon / Less Than
TKB_ScrollLock = 0x4600,			//!< 0x46 Scrolllock
TKB_Shift_Left = 0x2a00,			//!< 0x2A Left Shift
TKB_Shift_Right = 0x3600,			//!< 0x36 Right Shift
TKB_Single_Quote = 0x2827,			//!< 0x28 Single Quote / Double Quote
TKB_Space_Bar = 0x3920,				//!< 0x39 Spacebar
TKB_Square_Bracket_Open = 0x1a5b,	//!< 0x1A Open Square Bracket 
TKB_Square_Bracket_Close = 0x1b5d, 	//!< 0x1B Closed Square Bracket
TKB_Tab = 0x0f09,					//!< 0x0F Tab
TKB_Tilde = 0x2960,					//!< 0x29 Tilde
TKB_Back_Slash = 0x2b5c,			//!< 0x2B BackSlash (Used by VX1 to open menu)

// Function Keys

TKB_F1 = 0x3b00,					//!< 0x3B F1 function key
TKB_F2 = 0x3C00,					//!< 0x3C F2 function key
TKB_F3 = 0x3D00,					//!< 0x3D F3 function key
TKB_F4 = 0x3E00,					//!< 0x3E F4 function key
TKB_F5 = 0x3F00,					//!< 0x3F F5 function key
TKB_F6 = 0x4000,					//!< 0x40 F6 function key
TKB_F7 = 0x4100,					//!< 0x41 F7 function key
TKB_F8 = 0x4200,					//!< 0x42 F8 function key
TKB_F9 = 0x4300,					//!< 0x43 F9 function key
TKB_F10 = 0x4400,					//!< 0x44 F10 function key
TKB_F11 = 0x5700,					//!< 0x57 F11 function key
TKB_F12 = 0x5800,					//!< 0x58 F12 function key

// Numpad

TKB_NUMPAD_0 = 0x5230,				//!< 0x52 Numpad 0 key
TKB_NUMPAD_1 = 0x4F31,				//!< 0x4F Numpad 1 key
TKB_NUMPAD_2 = 0x5032,				//!< 0x50 Numpad 2 key	
TKB_NUMPAD_3 = 0x5133,				//!< 0x51 Numpad 3 key
TKB_NUMPAD_4 = 0x4B34,				//!< 0x4B Numpad 4 key
TKB_NUMPAD_5 = 0x4C35,				//!< 0x4C Numpad 5 key
TKB_NUMPAD_6 = 0x4D36,				//!< 0x4D Numpad 6 key
TKB_NUMPAD_7 = 0x4737,				//!< 0x47 Numpad 7 key
TKB_NUMPAD_8 = 0x4838,				//!< 0x48 Numpad 8 key
TKB_NUMPAD_9 = 0x4939,				//!< 0x49 Numpad 9 key
TKB_NUMPAD_Decimal = 0x532e,		//!< 0x53 Numpad decimal key
TKB_NUMPAD_Divide = 0xB500,			//!< 0xB5 Numpad divide key
TKB_NUMPAD_Multiply = 0x372a,		//!< 0x37 Numpad multiply key
TKB_NUMPAD_Minus = 0x4A2d,			//!< 0x4A Numpad minus key
TKB_NUMPAD_Plus = 0x4E2b,			//!< 0x4E Numpad plus key
TKB_NUMPAD_Enter = 0x9c00,			//!< 0x9C Numpad enter key

// Arrow Keys

TKB_Arrow_Right = 0xcd00,			//!< 0xCD Arrow Right    
TKB_Arrow_Left = 0xcb00,			//!< 0xCB Arrow Left
TKB_Arrow_Up = 0xC800,				//!< 0xC8 Arrow Up
TKB_Arrow_Down = 0xd000,			//!< 0xD0 Arrow Down
};


//! \enum VoxieMenu types used for creating custom touch screen menus
/**
* @see VoxieBox::menuAddItem()
* @see VoxieBox::updateMenu()
*
*/
enum VoxieMenuTypes { 
	MENU_TEXT = 0,							//!<  text (decoration only)
	MENU_LINE = 1,							//!<  line (decoration only)
	MENU_BUTTON_MIDDLE = 2,						//!<  push button in the middle of a group
	MENU_BUTTON_FIRST = MENU_BUTTON_MIDDLE + 1,	//!<  push button first in the group
	MENU_BUTTON_LAST  = MENU_BUTTON_MIDDLE + 2,	//!<  push button last in the group
	MENU_BUTTON_SINGLE = MENU_BUTTON_MIDDLE + 3,	//!<  push button single button
	MENU_HSLIDER = MENU_BUTTON_MIDDLE + 4,			//!<  horizontal slider
	MENU_VSLIDER = 7,						//!<  vertical slider
	MENU_EDIT = 8,							//!<  edit text box
	MENU_EDIT_DO = 9,						//!<  edit text box + activates next item on 'Enter'
	MENU_TOGGLE = 10,						//!<  combo box (w/o the drop down). Useful for saving space in dialog.
											//!<  Specify multiple strings in 'st' using \r as separator.
	MENU_PICKFILE = 11						//!<  File selector. Specify type in 2nd strig. Example "Browse\r*.kv6"
};

//! \enum Voxie capture modes used to determine capture type. 
/**
* @see VoxieBox::captureVolume()
*
*/

enum VolumeCaptureModes { 
	VOLCAP_OFF = 0, 
	VOLCAP_FRAME_PLY, 
	VOLCAP_FRAME_PNG, 
	VOLCAP_FRAME_REC, 
	VOLCAP_VIDEO_REC, 
	VOLCAP_FRAME_VCB, 
	VOLCAP_VIDEO_VCBZIP
};

/** Default touchkey taken from Voxiebox source*/ 
const touchkey_t default_touchkey[] =
{
	(char*)"`"    ,  0, 80, 64, 64,0x605040,-1,0x2960,
	(char*)"1"    , 68, 80, 64, 64,0x605040,-1,0x0231,
	(char*)"2"    ,136, 80, 64, 64,0x605040,-1,0x0332,
	(char*)"3"    ,204, 80, 64, 64,0x605040,-1,0x0433,
	(char*)"4"    ,272, 80, 64, 64,0x605040,-1,0x0534,
	(char*)"5"    ,340, 80, 64, 64,0x605040,-1,0x0635,
	(char*)"6"    ,408, 80, 64, 64,0x605040,-1,0x0736,
	(char*)"7"    ,476, 80, 64, 64,0x605040,-1,0x0837,
	(char*)"8"    ,544, 80, 64, 64,0x605040,-1,0x0938,
	(char*)"9"    ,612, 80, 64, 64,0x605040,-1,0x0a39,
	(char*)"0"    ,680, 80, 64, 64,0x605040,-1,0x0b30,
	(char*)"-"    ,748, 80, 64, 64,0x605040,-1,0x0c2d,
	(char*)"="    ,816, 80, 64, 64,0x605040,-1,0x0d3d,
	(char*)"Backspace",884, 80,139, 64,0x405060,-1,0x0e08,

	(char*)"Tab"  ,  0,148, 98, 64,0x605040,-1,0x0f09,
	(char*)"Q"    ,102,148, 64, 64,0x605040,-1,0x1071,
	(char*)"W"    ,170,148, 64, 64,0x605040,-1,0x1177,
	(char*)"E"    ,238,148, 64, 64,0x605040,-1,0x1265,
	(char*)"R"    ,306,148, 64, 64,0x605040,-1,0x1372,
	(char*)"T"    ,374,148, 64, 64,0x605040,-1,0x1474,
	(char*)"Y"    ,442,148, 64, 64,0x605040,-1,0x1579,
	(char*)"U"    ,510,148, 64, 64,0x605040,-1,0x1675,
	(char*)"I"    ,578,148, 64, 64,0x605040,-1,0x1769,
	(char*)"O"    ,646,148, 64, 64,0x605040,-1,0x186f,
	(char*)"P"    ,714,148, 64, 64,0x605040,-1,0x1970,
	(char*)"["    ,782,148, 64, 64,0x605040,-1,0x1a5b,
	(char*)"]"    ,850,148, 64, 64,0x605040,-1,0x1b5d,
	(char*)"\\"   ,918,148,105, 64,0x605040,-1,0x2b5c,

	(char*)"Caps",0,216, 98, 64,0x405060,-1,0x3a00,
	(char*)"A"    ,124,216, 64, 64,0x605040,-1,0x1e61,
	(char*)"S"    ,192,216, 64, 64,0x605040,-1,0x1f73,
	(char*)"D"    ,260,216, 64, 64,0x605040,-1,0x2074,
	(char*)"F"    ,328,216, 64, 64,0x605040,-1,0x2166,
	(char*)"G"    ,396,216, 64, 64,0x605040,-1,0x2267,
	(char*)"H"    ,464,216, 64, 64,0x605040,-1,0x2368,
	(char*)"J"    ,532,216, 64, 64,0x605040,-1,0x246a,
	(char*)"K"    ,600,216, 64, 64,0x605040,-1,0x256b,
	(char*)"L"    ,668,216, 64, 64,0x605040,-1,0x266c,
	(char*)";"    ,736,216, 64, 64,0x605040,-1,0x273b,
	(char*)"'"    ,804,216, 64, 64,0x605040,-1,0x2827,
	(char*)"Enter",872,216,151, 64,0x605040,-1,0x1c0d,

	(char*)"Shift",  0,284,154, 64,0x405060,-1,0x2a00,
	(char*)"Z"    ,158,284, 64, 64,0x605040,-1,0x2c7a,
	(char*)"X"    ,226,284, 64, 64,0x605040,-1,0x2d78,
	(char*)"C"    ,294,284, 64, 64,0x605040,-1,0x2e63,
	(char*)"V"    ,362,284, 64, 64,0x605040,-1,0x2f76,
	(char*)"B"    ,430,284, 64, 64,0x605040,-1,0x3062,
	(char*)"N"    ,498,284, 64, 64,0x605040,-1,0x316e,
	(char*)"M"    ,566,284, 64, 64,0x605040,-1,0x326d,
	(char*)","    ,634,284, 64, 64,0x605040,-1,0x332c,
	(char*)"."    ,702,284, 64, 64,0x605040,-1,0x342e,
	(char*)"/"    ,770,284, 64, 64,0x605040,-1,0x352f,
	(char*)"Shift",838,284,185, 64,0x405060,-1,0x3600,

	(char*)"Ctrl" ,  0,352,100, 64,0x405060,-1,0x1d00,
	(char*)"Alt"  ,174,352,100, 64,0x405060,-1,0x3800,
	(char*)" "    ,278,352,468, 64,0x605040,-1,0x3920,
	(char*)"Alt"  ,750,352,100, 64,0x405060,-1,0xb800,
	(char*)"Ctrl" ,924,352, 99, 64,0x405060,-1,0x9d00,

	(char*)"LMB"  ,  0,432,116,167,0x406040,-1,0x0001, //LBut
	(char*)"Touch Pad",120,432,216,167,0x406040,-1,0x0000, //Mouse
	(char*)"RMB"  ,340,432, 96,167,0x406040,-1,0x0002, //RBut
 //(char*)"MMB"  ,440,432, 96,167,0x406040,-1,0x0003, //MBut

	(char*)"Ins"  ,510,464, 64, 64,0x405060,-1,0xd200,
	(char*)"Home" ,578,464, 64, 64,0x405060,-1,0xc700,
	(char*)"PgUp" ,646,464, 64, 64,0x405060,-1,0xc900,
	(char*)"Del"  ,510,532, 64, 64,0x405060,-1,0xd300,
	(char*)"End"  ,578,532, 64, 64,0x405060,-1,0xcf00,
	(char*)"PgDn" ,646,532, 64, 64,0x405060,-1,0xd100,

	(char*)"ESC"  ,750,420, 64, 64,0x405060,-1,0x011b,

	(char*)"\xc8" ,860,432, 80, 80,0x405060,-1,0xc800, // hak to show graphical arrow
	(char*)"\xcb" ,776,512, 80, 80,0x405060,-1,0xcb00, // hak to show graphical arrow
	(char*)"\xd0" ,860,512, 80, 80,0x405060,-1,0xd000, // hak to show graphical arrow 
	(char*)"\xcd" ,944,512, 79, 80,0x405060,-1,0xcd00, // hak to show graphical arrow
};

//! \enum musical notes frequencies, can be used with a custom audio call back function to play musical notes.
enum MUSICAL_NOTES {
	B1 = 6173,

	C2 = 6540,
	Db2 = 6929,
	D2 = 7341,
	Eb2 = 7778,
	E2 = 8240,
	F2 = 8730,
	Gb2 = 9249,
	G2 = 9799,
	Ab2 = 10382,
	A2 = 11000,
	Bb2 = 11654,
	B2 = 12347,

	C3 = 13081,
	Db3 = 13859,
	D3 = 14683,
	Eb3 = 15556,
	E3 = 16481,
	F3 = 17461,
	Gb3 = 18499,
	G3 = 19599,
	Ab3 = 20765,
	A3 = 22000,
	Bb3 = 23308,
	B3 = 24694,

	C4 = 26162,
	Db4 = 27718,
	D4 = 29366,
	Eb4 = 31112,
	E4 = 32962,
	F4 = 34922,
	Gb4 = 36999,
	G4 = 39199,
	Ab4 = 41530,
	A4 = 44000,
	Bb4 = 46616,
	B4 = 49388,

	C5 = 52325,
	Db5 = 55436,
	D5 = 58732,
	Eb5 = 62225,
	E5 = 65922,
	F5 = 69845,
	Gb5 = 73998,
	G5 = 78399,
	Ab5 = 83060,
	A5 = 88000,
	Bb5 = 93232,
	B5 = 98776,

	C6 = 104650,
	Db6 = 110873,
	D6 = 117465,
	Eb6 = 124450,
	E6 = 131851,
	F6 = 139691,
	Gb6 = 147997,
	G6 = 156798,
	Ab6 = 166121,
	A6 = 176000,
	Bb6 = 186465,
	B6 = 197553,

	C7 = 209300,
	Db7 = 221746,
	D7 = 234931,
	Eb7 = 248901,
	E7 = 263702,
	F7 = 279382,
	Gb7 = 295995,
	G7 = 313596,
	Ab7 = 332243,
	A7 = 352000,
	Bb7 = 372931,
	B7 = 395106,

	C8 = 418600,
	Db8 = 443492,
	D8 = 469863,
	Eb8 = 497803,
	E8 = 527404,
	F8 = 558765,
	Gb8 = 591991,
	G8 = 627192,
	Ab8 = 664487,
	A8 = 704000,
	Bb8 = 745862,
	B8 = 790213,

};

#endif
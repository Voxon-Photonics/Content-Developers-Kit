from enum import Enum

        ## this functions initalises all the scan codes for various input types and enums used in the system
class VXScanCode(Enum):
        
        ## Joy / Gamepad the bit value button codes to work with Xbox / Game controllers  
        
        JOY_DPAD_UP			    = (0) 	# bit 0 dec value 1			Digital Dpad Up				
        JOY_DPAD_DOWN			= (1) 	# bit 1 dec value 2			Digital Dpad Down 
        JOY_DPAD_LEFT			= (2) 	# bit 2 dec value 4			Digital Dpad Left
        JOY_DPAD_RIGHT			= (3) 	# bit 3 dec value 8			Digital Dpad Right
        JOY_START				= (4) 	# bit 4 dec value 16		Start Button
        JOY_BACK				= (5) 	# bit 5 dec value 32		Back Button
        JOY_LEFT_THUMB			= (6) 	# bit 6 dec value 64		Left Thumb Stick Button (when you press 'down' on the left analog stick)
        JOY_RIGHT_THUMB		    = (7) 	# bit 7 dec value 128		Right Thumb Stick Button (when you press 'down' on the right analog stick)
        JOY_LEFT_SHOULDER		= (8)	# bit 8 dec value 256		Left Shoulder Bumper Button - not the Shoulder triggers are analog 
        JOY_RIGHT_SHOULDER		= (9)	# bit 9 dec value 512		Right Shoulder Bumper Button - not the Shoulder triggers are analog 
        JOY_BUT_A				= (12)	# bit 12 dec value 1,024	The 'A' Button on a standard Xbox Controller
        JOY_BUT_B				= (13)	# bit 13 dec value 2,048	The 'B' Button on a standard Xbox Controller
        JOY_BUT_X				= (14)	# bit 14 dec value 4,096	The 'X' Button on a standard Xbox Controller
        JOY_BUT_Y				= (15)	# bit  15 dec value 8,192	The 'Y' Button on a standard Xbox Controller

        # enum Joy Controller Axis Enum helps identify which analog input is being used 
        
        JOY_STICK_LEFT_X    = (0)			# Analog Left Stick X axis
        JOY_STICK_LEFT_Y    = (1)			# Analog Left Stick Y axis
        JOY_STICK_RIGHT_X   = (2)			# Analog Right Stick X axis
        JOY_STICK_RIGHT_Y   = (3)			# Analog Right Stick Y axis
        
        JOY_TRIG_LEFT   = (0)	# 0 Analog Left Shoulder trigger 
        JOY_TRIG_RIGHT  = (1)	# 1 Analog Right Shoulder trigger

        ## Mouse

        MOUSE_BUT_LEFT      = (0)		# bit 0 value 1			Left Mouse Button
        MOUSE_BUT_RIGHT     = (1)		# bit 1 value 2			Right Mouse Button
        MOUSE_BUT_MIDDLE    = (2)	    # bit 2 value 4			Middle Mouse Button

        MOUSE_AXIS_X = (0)
        MOUSE_AXIS_Y = (1)
        MOUSE_AXIS_Z = (2)
        
        ## Space Nav
        
        NAV_AXIS_DIR_X = (0)			
        NAV_AXIS_DIR_Y = (1)		
        NAV_AXIS_DIR_Z = (2)    
        NAV_AXIS_ANG_X = (3)			
        NAV_AXIS_ANG_Y = (4)		
        NAV_AXIS_ANG_Z = (5)            
        NAV_BUT_LEFT = (0)			# bit 0 value 1 Left  SpaceNav Button
        NAV_BUT_RIGHT = (1)			# bit 1 value 2 Right SpaceNav Button
        
        ## ReadKey Control / Shifts

        READKEY_LEFT_SHIFT  = (0)    # bit 0 value Left  Shift
        READKEY_RIGHT_SHIFT = (1)    # bit 0 value Left  Shift
        READKEY_LEFT_CTRL = (2)    # bit 0 value Left  Shift
        READKEY_RIGHT_CTRL = (3)    # bit 0 value Left  Shift
        READKEY_LEFT_ALT = (4)    # bit 0 value Left  Shift
        READKEY_RIGHT_ALT = (5)    # bit 0 value Left  Shift

       
        ## define Key types
        
        KEY_Empty = (0x00)				# 0x00 Empty state 
        KEY_Escape = (0x01)			# 0x01 'Esc' / Escape key
        
           
        KEY_1 = (0x02)					# (0x02 Number 1 key
        KEY_2 = (0x03)					# (0x03 Number 2 key
        KEY_3 = (0x04)					# (0x04 Number 3 key
        KEY_4 = (0x05)					# (0x05 Number 4 key
        KEY_5 = (0x06)					# (0x06 Number 5 key
        KEY_6 = (0x07)					# (0x07 Number 6 key
        KEY_7 = (0x08)					# (0x08 Number 7 key
        KEY_8 = (0x09)					# (0x09 Number 8 key
        KEY_9 = (0x0A)					# (0x0A Number 9 key
        KEY_0 = (0x0B)					# (0x0B Number 0 key
        
        # Alpha Keys
        
        KEY_A = (0x1E)					# (0x1E 'A' key
        KEY_B = (0x30)					# (0x30 'B' key
        KEY_C = (0x2E)					# (0x2E 'C' key
        KEY_D = (0x20)					# (0x20 'D' key
        KEY_E = (0x12)					# (0x12 'E' key
        KEY_F = (0x21)					# (0x21 'F' key
        KEY_G = (0x22)					# (0x22 'G' key
        KEY_H = (0x23)					# (0x23 'H' key
        KEY_I = (0x17)					# (0x17 'I' key
        KEY_J = (0x24)					# (0x24 'J' key
        KEY_K = (0x25)					# (0x25 'K' key
        KEY_L = (0x26)					# (0x26 'L' key
        KEY_M = (0x32)					# (0x32 'M' key
        KEY_N = (0x31)					# (0x31 'N' key
        KEY_O = (0x18)					# (0x18 'O' key
        KEY_P = (0x19)					# (0x19 'P' key
        KEY_Q = (0x10)					# (0x10 'Q' key
        KEY_R = (0x13)					# (0x13 'R' key
        KEY_S = (0x1F)					# (0x1F 'S' key
        KEY_T = (0x14)					# (0x14 'T' key
        KEY_U = (0x16)					# (0x16 'U' key
        KEY_V = (0x2F)					# (0x2F 'V' key
        KEY_W = (0x11)					# (0x11 'W' key
        KEY_X = (0x2D)					# (0x2D 'X' key
        KEY_Y = (0x15)					# (0x15 'Y' key
        KEY_Z = (0x2C)					# (0x2C 'Z' key
        
        # Special Keys
        
        KEY_Alt_Left = (0x38)				# (0x38 Left Alt
        KEY_Alt_Right = (0xB8)			# (0xB8 Right Alt  
        KEY_Backspace = (0x0E)			# (0x0E Backspace
        KEY_CapsLock = (0x3A)				# (0x3A Capslock
        KEY_Comma = (0x33)				# (0x33 Comma
        KEY_Control_Left = (0x1D)			# (0x1D Left Control
        KEY_Control_Right = (0x9D)		# (0x9D Right Control
        KEY_Delete = (0xD3)				# (0xD3 Delete
        KEY_Forward_Slash = (0x35)		# (0x35 Divide
        KEY_Full_Stop = (0x34)			# (0x34 Fullstop / Greater Than
        KEY_End = (0xCF)					# (0xCF End
        KEY_Enter = (0x1C)				# (0x1C Enter
        KEY_Equals = (0x0D)				# (0x0D Equals
        KEY_Home = (0xC7)					# (0xC7 Home
        KEY_Insert = (0xD2)				# (0xD2 Insert
        KEY_Minus = (0x0C)				# (0x0C Minus / Dash 
        KEY_Numlock = (0xC5)				# (0xC5 Numlock
        KEY_Page_Down = (0xD1)			# (0xD1 Page Down
        KEY_Page_Up = (0xC9)				# (0xC9 Page Up
        KEY_Pause = (0x45)				# (0x45 Pause
        KEY_Print_Screen = (0xB7)			# (0xB7 Print_Screen
        KEY_Secondary_Action = (0xDD)		# (0xDD Secondary Action (not on all keyboards)
        KEY_Semicolon = (0x27)			# (0x27 Semicolon / Less Than
        KEY_ScrollLock = (0x46)			# (0x46 Scrolllock
        KEY_Shift_Left = (0x2A)			# (0x2A Left Shift
        KEY_Shift_Right = (0x36)			# (0x36 Right Shift
        KEY_Single_Quote = (0x28)			# (0x28 Single Quote / Double Quote
        KEY_Space_Bar = (0x39)			# (0x39 Spacebar
        KEY_Square_Bracket_Open = (0x1A)	# (0x1A Open Square Bracket 
        KEY_Square_Bracket_Close = (0x1B) # (0x1B Closed Square Bracket
        KEY_Tab = (0x0F)					# (0x0F Tab
        KEY_Tilde = (0x29)				# (0x29 Tilde
        KEY_Back_Slash = (0x2B)			# (0x2B BackSlash (Used by VX1 to open menu)
        
        # Function Keys
        
        KEY_F1 = (0x3B)					# (0x3B F1 function key
        KEY_F2 = (0x3C)					# (0x3C F2 function key
        KEY_F3 = (0x3D)					# (0x3D F3 function key
        KEY_F4 = (0x3E)					# (0x3E F4 function key
        KEY_F5 = (0x3F)					# (0x3F F5 function key
        KEY_F6 = (0x40)					# (0x40 F6 function key
        KEY_F7 = (0x41)					# (0x41 F7 function key
        KEY_F8 = (0x42)					# (0x42 F8 function key
        KEY_F9 = (0x43)					# (0x43 F9 function key
        KEY_F10 = (0x44)				# (0x44 F10 function key
        KEY_F11 = (0x57)				# (0x57 F11 function key
        KEY_F12 = (0x58)				# (0x58 F12 function key
        
        # Numpad
        
        KEY_NUMPAD_0 = (0x52)				# (0x52 Numpad 0 key
        KEY_NUMPAD_1 = (0x4F)				# (0x4F Numpad 1 key
        KEY_NUMPAD_2 = (0x50)				# (0x50 Numpad 2 key	
        KEY_NUMPAD_3 = (0x51)				# (0x51 Numpad 3 key
        KEY_NUMPAD_4 = (0x4B)				# (0x4B Numpad 4 key
        KEY_NUMPAD_5 = (0x4C)				# (0x4C Numpad 5 key
        KEY_NUMPAD_6 = (0x4D)				# (0x4D Numpad 6 key
        KEY_NUMPAD_7 = (0x47)				# (0x47 Numpad 7 key
        KEY_NUMPAD_8 = (0x48)				# (0x48 Numpad 8 key
        KEY_NUMPAD_9 = (0x49)				# (0x49 Numpad 9 key
        KEY_NUMPAD_Decimal = (0x53)		    # (0x53 Numpad decimal key
        KEY_NUMPAD_Divide = (0xB5)		    # (0xB5 Numpad divide key
        KEY_NUMPAD_Multiply = (0x37)		# (0x37 Numpad multiply key
        KEY_NUMPAD_Minus = (0x4A)			# (0x4A Numpad minus key
        KEY_NUMPAD_Plus = (0x4E)			# (0x4E Numpad plus key
        KEY_NUMPAD_Enter = (0x9C)			# (0x9C Numpad enter key
        
        # Arrow Keys
        
        KEY_Arrow_Right = (0xCD)			# (0xCD Arrow Right
        KEY_Arrow_Left = (0xCB)			    # (0xCB Arrow Left
        KEY_Arrow_Up = (0xC8)				# (0xC8 Arrow Up
        KEY_Arrow_Down = (0xD0)			    # (0xD0 Arrow Down


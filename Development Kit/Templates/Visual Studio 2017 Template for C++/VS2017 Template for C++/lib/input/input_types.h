#pragma once

const int MAX_CONTROLLERS = 4;

enum ControllerAxis {
	LEFT_STICK_X,
	LEFT_STICK_Y,
	RIGHT_STICK_X,
	RIGHT_STICK_Y,
};

enum ControllerTrigger {
	LEFT_TRIGGER,
	RIGHT_TRIGGER,
};

enum MouseButton {
	LEFT_BUTTON,
	RIGHT_BUTTON,
	MIDDLE_BUTTON,
};


enum Keys
{
	K_ = 0x00,
	K_Escape = 0x01,
	K_1 = 0x02,
	K_2 = 0x03,
	K_3 = 0x04,
	K_4 = 0x05,
	K_5 = 0x06,
	K_6 = 0x07,
	K_7 = 0x08,
	K_8 = 0x09,
	K_9 = 0x0A,
	K_0 = 0x0B,

	K_A = 0x1E,
	K_B = 0x30,
	K_C = 0x2E,
	K_D = 0x20,
	K_E = 0x12,
	K_F = 0x21,
	K_G = 0x22,
	K_H = 0x23,
	K_I = 0x17,
	K_J = 0x24,
	K_K = 0x25,
	K_L = 0x26,
	K_M = 0x32,
	K_N = 0x31,
	K_O = 0x18,
	K_P = 0x19,
	K_Q = 0x10,
	K_R = 0x13,
	K_S = 0x1F,
	K_T = 0x14,
	K_U = 0x16,
	K_V = 0x2F,
	K_W = 0x11,
	K_X = 0x2D,
	K_Y = 0x15,
	K_Z = 0x2C,

	K_Alt_Left = 0x38,
	K_Alt_Right = 0xB8,
	K_Backspace = 0x0E,
	K_CapsLock = 0x3A,
	K_Comma = 0x33,
	K_Control_Left = 0x1D,
	K_Control_Right = 0x9D,
	K_Delete = 0xD3,
	K_Divide = 0x35,
	K_Dot = 0x34,
	K_End = 0xCF,
	K_Enter = 0x1C,
	K_Equals = 0x0D,
	K_Home = 0xC7,
	K_Insert = 0xD2,
	K_Minus = 0x0C,
	K_NumLock = 0x45,
	K_PageDown = 0xD1,
	K_PageUp = 0xC9,
	K_Pause = 0xC5,
	K_PrintScreen = 0xB7,
	K_SecondaryAction = 0xDD,
	K_SemiColon = 0x27,
	K_ScrollLock = 0x46,
	K_Shift_Left = 0x2A,
	K_Shift_Right = 0x36,
	K_SingleQuote = 0x28,
	K_Space = 0x39,
	K_SquareBracket_Open = 0x1A,
	K_SquareBracket_Close = 0x1B,
	K_Tab = 0x0F,
	K_Tilde = 0x29,
	//BackSlash = 0x2B, (Owned by VX1)

	K_F1 = 0x3B,
	K_F2 = 0x3C,
	K_F3 = 0x3D,
	K_F4 = 0x3E,
	K_F5 = 0x3F,
	K_F6 = 0x40,
	K_F7 = 0x41,
	K_F8 = 0x42,
	K_F9 = 0x43,
	K_F10 = 0x44,
	K_F11 = 0x57,
	K_F12 = 0x58,

	K_NUMPAD_Divide = 0xB5,
	K_NUMPAD_Multiply = 0x37,
	K_NUMPAD_Minux = 0x4A,
	K_NUMPAD_Plus = 0x4E,
	K_NUMPAD_Enter = 0x9C,

	K_NUMPAD_0 = 0x52,
	K_NUMPAD_1 = 0x4F,
	K_NUMPAD_2 = 0x50,
	K_NUMPAD_3 = 0x51,
	K_NUMPAD_4 = 0x4B,
	K_NUMPAD_5 = 0x4C,
	K_NUMPAD_6 = 0x4D,
	K_NUMPAD_7 = 0x47,
	K_NUMPAD_8 = 0x48,
	K_NUMPAD_9 = 0x49,
	K_NUMPAD_Dot = 0x53,

	K_Arrow_Right = 0xCD,
	K_Arrow_Left = 0xCB,
	K_Arrow_Up = 0xC8,
	K_Arrow_Down = 0xD0,
};

enum Key_Modifier {
	NONE,
	CTRL,
	ALT,
	SHIFT
};
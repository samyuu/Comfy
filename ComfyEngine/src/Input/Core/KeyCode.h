#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Input
{
	using KeyState = i32;
	enum KeyState_Enum : KeyState
	{
		KeyState_Release = 0,
		KeyState_Press = 1,
		KeyState_Repeat = 2,
	};

	using KeyCode = i32;
	enum KeyCode_Enum : KeyCode
	{
		KeyCode_Unknown = -1,
		KeyCode_MouseLeft = 0x01,
		KeyCode_MouseRight = 0x02,
		KeyCode_MouseMiddle = 0x04,
		KeyCode_MouseX1 = 0x05,
		KeyCode_MouseX2 = 0x06,
		KeyCode_Backspace = 0x08,
		KeyCode_Tab = 0x09,
		KeyCode_Clear = 0x0C,
		KeyCode_Enter = 0x0D,
		KeyCode_Shift = 0x10,
		KeyCode_Control = 0x11,
		KeyCode_Alt = 0x12,
		KeyCode_Pause = 0x13,
		KeyCode_CapsLock = 0x14,
		KeyCode_Escape = 0x1B,
		KeyCode_Space = 0x20,
		KeyCode_Prior = 0x21,
		KeyCode_Next = 0x22,
		KeyCode_End = 0x23,
		KeyCode_Home = 0x24,
		KeyCode_Left = 0x25,
		KeyCode_Up = 0x26,
		KeyCode_Right = 0x27,
		KeyCode_Down = 0x28,
		KeyCode_Select = 0x29,
		KeyCode_Print = 0x2A,
		KeyCode_Insert = 0x2D,
		KeyCode_Delete = 0x2E,
		KeyCode_Help = 0x2F,
		KeyCode_0 = '0',
		KeyCode_1 = '1',
		KeyCode_2 = '2',
		KeyCode_3 = '3',
		KeyCode_4 = '4',
		KeyCode_5 = '5',
		KeyCode_6 = '6',
		KeyCode_7 = '7',
		KeyCode_8 = '8',
		KeyCode_9 = '9',
		KeyCode_A = 'A',
		KeyCode_B = 'B',
		KeyCode_C = 'C',
		KeyCode_D = 'D',
		KeyCode_E = 'E',
		KeyCode_F = 'F',
		KeyCode_G = 'G',
		KeyCode_H = 'H',
		KeyCode_I = 'I',
		KeyCode_J = 'J',
		KeyCode_K = 'K',
		KeyCode_L = 'L',
		KeyCode_M = 'M',
		KeyCode_N = 'N',
		KeyCode_O = 'O',
		KeyCode_P = 'P',
		KeyCode_Q = 'Q',
		KeyCode_R = 'R',
		KeyCode_S = 'S',
		KeyCode_T = 'T',
		KeyCode_U = 'U',
		KeyCode_V = 'V',
		KeyCode_W = 'W',
		KeyCode_X = 'X',
		KeyCode_Y = 'Y',
		KeyCode_Z = 'Z',
		KeyCode_LeftWin = 0x5B,
		KeyCode_RightWin = 0x5C,
		KeyCode_Apps = 0x5D,
		KeyCode_Sleep = 0x5F,
		KeyCode_Numpad0 = 0x60,
		KeyCode_Numpad1 = 0x61,
		KeyCode_Numpad2 = 0x62,
		KeyCode_Numpad3 = 0x63,
		KeyCode_Numpad4 = 0x64,
		KeyCode_Numpad5 = 0x65,
		KeyCode_Numpad6 = 0x66,
		KeyCode_Numpad7 = 0x67,
		KeyCode_Numpad8 = 0x68,
		KeyCode_Numpad9 = 0x69,
		KeyCode_Multiply = 0x6A,
		KeyCode_Add = 0x6B,
		KeyCode_Separator = 0x6C,
		KeyCode_Subtract = 0x6D,
		KeyCode_Decimal = 0x6E,
		KeyCode_Divide = 0x6F,
		KeyCode_F1 = 0x70,
		KeyCode_F2 = 0x71,
		KeyCode_F3 = 0x72,
		KeyCode_F4 = 0x73,
		KeyCode_F5 = 0x74,
		KeyCode_F6 = 0x75,
		KeyCode_F7 = 0x76,
		KeyCode_F8 = 0x77,
		KeyCode_F9 = 0x78,
		KeyCode_F10 = 0x79,
		KeyCode_F11 = 0x7A,
		KeyCode_F12 = 0x7B,
		KeyCode_F13 = 0x7C,
		KeyCode_F14 = 0x7D,
		KeyCode_F15 = 0x7E,
		KeyCode_F16 = 0x7F,
		KeyCode_F17 = 0x80,
		KeyCode_F18 = 0x81,
		KeyCode_F19 = 0x82,
		KeyCode_F20 = 0x83,
		KeyCode_F21 = 0x84,
		KeyCode_F22 = 0x85,
		KeyCode_F23 = 0x86,
		KeyCode_F24 = 0x87,
		KeyCode_NumLock = 0x90,
		KeyCode_Scroll = 0x91,
		KeyCode_LeftShift = 0xA0,
		KeyCode_RightShift = 0xA1,
		KeyCode_LeftControl = 0xA2,
		KeyCode_RightControl = 0xA3,
		KeyCode_LeftAlt = 0xA4,
		KeyCode_RightAlt = 0xA5,
		KeyCode_BrowserBack = 0xA6,
		KeyCode_BrowserForward = 0xA7,
		KeyCode_BrowserRefresh = 0xA8,
		KeyCode_BrowserStop = 0xA9,
		KeyCode_BrowserSearch = 0xAA,
		KeyCode_BrowserFavorites = 0xAB,
		KeyCode_BrowserHome = 0xAC,
		KeyCode_VolumeMute = 0xAD,
		KeyCode_VolumeDown = 0xAE,
		KeyCode_VolumeUp = 0xAF,
		KeyCode_MediaNextTrack = 0xB0,
		KeyCode_MediaPrevTrack = 0xB1,
		KeyCode_MediaStop = 0xB2,
		KeyCode_MediaPlayPause = 0xB3,
		KeyCode_LaunchMail = 0xB4,
		KeyCode_LaunchMediaSelect = 0xB5,
		KeyCode_LaunchApp1 = 0xB6,
		KeyCode_LaunchApp2 = 0xB7,
		KeyCode_OEM1 = 0xBA,
		KeyCode_OEMPlus = 0xBB,
		KeyCode_OEMComma = 0xBC,
		KeyCode_OEMMinus = 0xBD,
		KeyCode_OEMPeriod = 0xBE,
		KeyCode_OEM2 = 0xBF,
		KeyCode_OEM3 = 0xC0,
		KeyCode_OEM4 = 0xDB,
		KeyCode_OEM5 = 0xDC,
		KeyCode_OEM6 = 0xDD,
		KeyCode_OEM7 = 0xDE,
		KeyCode_OEM8 = 0xDF,
		KeyCode_OEM102 = 0xE2,
		KeyCode_IMEProcessKey = 0xE5,
		KeyCode_Packet = 0xE7,
		KeyCode_Attn = 0xF6,
		KeyCode_CrSel = 0xF7,
		KeyCode_ExSel = 0xF8,
		KeyCode_EraseEOF = 0xF9,
		KeyCode_Play = 0xFA,
		KeyCode_Zoom = 0xFB,
		KeyCode_NoName = 0xFC,
		KeyCode_PA1 = 0xFD,
		KeyCode_OEMClear = 0xFE,

		KeyCode_Count,
		KeyCode_MouseFirst = KeyCode_MouseLeft,
		KeyCode_MouseLast = KeyCode_MouseX2,
		KeyCode_KeyboardFirst = KeyCode_Backspace,
		KeyCode_KeyboardLast = KeyCode_Count,
	};

	constexpr const char* GetKeyCodeName(const KeyCode keyCode)
	{
		if (keyCode >= KeyCode_Count)
			return nullptr;

		switch (keyCode)
		{
		case KeyCode_Unknown: return "Unknown";
		case KeyCode_MouseLeft: return "Mouse Left";
		case KeyCode_MouseRight: return "Mouse Right";
		case KeyCode_MouseMiddle: return "Mouse Middle";
		case KeyCode_MouseX1: return "Mouse X1";
		case KeyCode_MouseX2: return "Mouse X2";
		case KeyCode_Backspace: return "Backspace";
		case KeyCode_Tab: return "Tab";
		case KeyCode_Clear: return "Clear";
		case KeyCode_Enter: return "Enter";
		case KeyCode_Shift: return "Shift";
		case KeyCode_Control: return "Control";
		case KeyCode_Alt: return "Alt";
		case KeyCode_Pause: return "Pause";
		case KeyCode_CapsLock: return "Caps Lock";
		case KeyCode_Escape: return "Escape";
		case KeyCode_Space: return "Space";
		case KeyCode_Prior: return "Prior";
		case KeyCode_Next: return "Next";
		case KeyCode_End: return "End";
		case KeyCode_Home: return "Home";
		case KeyCode_Left: return "Left";
		case KeyCode_Up: return "Up";
		case KeyCode_Right: return "Right";
		case KeyCode_Down: return "Down";
		case KeyCode_Select: return "Select";
		case KeyCode_Print: return "Print";
		case KeyCode_Insert: return "Insert";
		case KeyCode_Delete: return "Delete";
		case KeyCode_Help: return "Help";
		case KeyCode_0: return "0";
		case KeyCode_1: return "1";
		case KeyCode_2: return "2";
		case KeyCode_3: return "3";
		case KeyCode_4: return "4";
		case KeyCode_5: return "5";
		case KeyCode_6: return "6";
		case KeyCode_7: return "7";
		case KeyCode_8: return "8";
		case KeyCode_9: return "9";
		case KeyCode_A: return "A";
		case KeyCode_B: return "B";
		case KeyCode_C: return "C";
		case KeyCode_D: return "D";
		case KeyCode_E: return "E";
		case KeyCode_F: return "F";
		case KeyCode_G: return "G";
		case KeyCode_H: return "H";
		case KeyCode_I: return "I";
		case KeyCode_J: return "J";
		case KeyCode_K: return "K";
		case KeyCode_L: return "L";
		case KeyCode_M: return "M";
		case KeyCode_N: return "N";
		case KeyCode_O: return "O";
		case KeyCode_P: return "P";
		case KeyCode_Q: return "Q";
		case KeyCode_R: return "R";
		case KeyCode_S: return "S";
		case KeyCode_T: return "T";
		case KeyCode_U: return "U";
		case KeyCode_V: return "V";
		case KeyCode_W: return "W";
		case KeyCode_X: return "X";
		case KeyCode_Y: return "Y";
		case KeyCode_Z: return "Z";
		case KeyCode_LeftWin: return "Left Win";
		case KeyCode_RightWin: return "Right Win";
		case KeyCode_Apps: return "Apps";
		case KeyCode_Sleep: return "Sleep";
		case KeyCode_Numpad0: return "Numpad 0";
		case KeyCode_Numpad1: return "Numpad 1";
		case KeyCode_Numpad2: return "Numpad 2";
		case KeyCode_Numpad3: return "Numpad 3";
		case KeyCode_Numpad4: return "Numpad 4";
		case KeyCode_Numpad5: return "Numpad 5";
		case KeyCode_Numpad6: return "Numpad 6";
		case KeyCode_Numpad7: return "Numpad 7";
		case KeyCode_Numpad8: return "Numpad 8";
		case KeyCode_Numpad9: return "Numpad 9";
		case KeyCode_Multiply: return "Multiply";
		case KeyCode_Add: return "Add";
		case KeyCode_Separator: return "Separator";
		case KeyCode_Subtract: return "Subtract";
		case KeyCode_Decimal: return "Decimal";
		case KeyCode_Divide: return "Divide";
		case KeyCode_F1: return "F1";
		case KeyCode_F2: return "F2";
		case KeyCode_F3: return "F3";
		case KeyCode_F4: return "F4";
		case KeyCode_F5: return "F5";
		case KeyCode_F6: return "F6";
		case KeyCode_F7: return "F7";
		case KeyCode_F8: return "F8";
		case KeyCode_F9: return "F9";
		case KeyCode_F10: return "F10";
		case KeyCode_F11: return "F11";
		case KeyCode_F12: return "F12";
		case KeyCode_F13: return "F13";
		case KeyCode_F14: return "F14";
		case KeyCode_F15: return "F15";
		case KeyCode_F16: return "F16";
		case KeyCode_F17: return "F17";
		case KeyCode_F18: return "F18";
		case KeyCode_F19: return "F19";
		case KeyCode_F20: return "F20";
		case KeyCode_F21: return "F21";
		case KeyCode_F22: return "F22";
		case KeyCode_F23: return "F23";
		case KeyCode_F24: return "F24";
		case KeyCode_NumLock: return "Num Lock";
		case KeyCode_Scroll: return "Scroll";
		case KeyCode_LeftShift: return "Left Shift";
		case KeyCode_RightShift: return "Right Shift";
		case KeyCode_LeftControl: return "Left Control";
		case KeyCode_RightControl: return "Right Control";
		case KeyCode_LeftAlt: return "Left Alt";
		case KeyCode_RightAlt: return "Right Alt";
		case KeyCode_BrowserBack: return "Browser Back";
		case KeyCode_BrowserForward: return "Browser Forward";
		case KeyCode_BrowserRefresh: return "Browser Refresh";
		case KeyCode_BrowserStop: return "Browser Stop";
		case KeyCode_BrowserSearch: return "Browser Search";
		case KeyCode_BrowserFavorites: return "Browser Favorite";
		case KeyCode_BrowserHome: return "Browser Home";
		case KeyCode_VolumeMute: return "Volume Mute";
		case KeyCode_VolumeDown: return "Volume Down";
		case KeyCode_VolumeUp: return "Volume Up";
		case KeyCode_MediaNextTrack: return "Media Next Track";
		case KeyCode_MediaPrevTrack: return "Media Prev Track";
		case KeyCode_MediaStop: return "Media Stop";
		case KeyCode_MediaPlayPause: return "Media Play Pause";
		case KeyCode_LaunchMail: return "Launch Mail";
		case KeyCode_LaunchMediaSelect: return "Launch Media Select";
		case KeyCode_LaunchApp1: return "Launch App 1";
		case KeyCode_LaunchApp2: return "Launch App 2";
		case KeyCode_OEM1: return "OEM 1";
		case KeyCode_OEMPlus: return "Plus";
		case KeyCode_OEMComma: return "Comma";
		case KeyCode_OEMMinus: return "Minus";
		case KeyCode_OEMPeriod: return "Period";
		case KeyCode_OEM2: return "OEM 2";
		case KeyCode_OEM3: return "OEM 3";
		case KeyCode_OEM4: return "OEM 4";
		case KeyCode_OEM5: return "OEM 5";
		case KeyCode_OEM6: return "OEM 6";
		case KeyCode_OEM7: return "OEM 7";
		case KeyCode_OEM8: return "OEM 8";
		case KeyCode_OEM102: return "OEM 102";
		case KeyCode_IMEProcessKey: return "IME Process Key";
		case KeyCode_Packet: return "Packet";
		case KeyCode_Attn: return "Attn";
		case KeyCode_CrSel: return "CrSel";
		case KeyCode_ExSel: return "ExSel";
		case KeyCode_EraseEOF: return "Erase EOF";
		case KeyCode_Play: return "Play";
		case KeyCode_Zoom: return "Zoom";
		case KeyCode_NoName: return "No Name";
		case KeyCode_PA1: return "PA1";
		case KeyCode_OEMClear: return "OEM Clear";

		default:
			assert(keyCode < KeyCode_Unknown);
			return nullptr;
		}
	}
}

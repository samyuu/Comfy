#include "KeyCode.h"
#include "CoreTypes.h"

namespace Comfy
{
	const char* GetKeyCodeName(const KeyCode keyCode)
	{
		if (keyCode == KeyCode_Unknown)
			return "Unknown";

		if (keyCode <= 0 || keyCode >= KeyCode_Count)
			return nullptr;

		switch (keyCode)
		{
		case KeyCode_MouseLeft:		return "Mouse Left";
		case KeyCode_MouseRight:	return "Mouse Right";
		case KeyCode_MouseMiddle:	return "Mouse Middle";
		case KeyCode_MouseX1:		return "Mouse X1";
		case KeyCode_MouseX2:		return "Mouse X2";
		case KeyCode_Backspace:		return "Backspace";
		case KeyCode_Tab:			return "Tab";
		case KeyCode_Clear:			return "Clear";
		case KeyCode_Enter:			return "Enter";
		case KeyCode_Shift:			return "Shift";
		case KeyCode_Control:		return "Control";
		case KeyCode_Alt:			return "Alt";
		case KeyCode_Pause:			return "Pause";
		case KeyCode_CapsLock:		return "Caps Lock";
		case KeyCode_Escape:		return "Escape";
		case KeyCode_Space:			return "Space";
		case KeyCode_Prior:			return "Prior";
		case KeyCode_Next:			return "Next";
		case KeyCode_End:			return "End";
		case KeyCode_Home:			return "Home";
		case KeyCode_Left:			return "Left";
		case KeyCode_Up:			return "Up";
		case KeyCode_Right:			return "Right";
		case KeyCode_Down:			return "Down";
		case KeyCode_Select:		return "Select";
		case KeyCode_Print:			return "Print";
		case KeyCode_Insert:		return "Insert";
		case KeyCode_Delete:		return "Delete";
		case KeyCode_Help:			return "Help";
		case KeyCode_0:				return "0";
		case KeyCode_1:				return "1";
		case KeyCode_2:				return "2";
		case KeyCode_3:				return "3";
		case KeyCode_4:				return "4";
		case KeyCode_5:				return "5";
		case KeyCode_6:				return "6";
		case KeyCode_7:				return "7";
		case KeyCode_8:				return "8";
		case KeyCode_9:				return "9";
		case KeyCode_A:				return "A";
		case KeyCode_B:				return "B";
		case KeyCode_C:				return "C";
		case KeyCode_D:				return "D";
		case KeyCode_E:				return "E";
		case KeyCode_F:				return "F";
		case KeyCode_G:				return "G";
		case KeyCode_H:				return "H";
		case KeyCode_I:				return "I";
		case KeyCode_J:				return "J";
		case KeyCode_K:				return "K";
		case KeyCode_L:				return "L";
		case KeyCode_M:				return "M";
		case KeyCode_N:				return "N";
		case KeyCode_O:				return "O";
		case KeyCode_P:				return "P";
		case KeyCode_Q:				return "Q";
		case KeyCode_R:				return "R";
		case KeyCode_S:				return "S";
		case KeyCode_T:				return "T";
		case KeyCode_U:				return "U";
		case KeyCode_V:				return "V";
		case KeyCode_W:				return "W";
		case KeyCode_X:				return "X";
		case KeyCode_Y:				return "Y";
		case KeyCode_Z:				return "Z";
		case KeyCode_LeftWin:		return "Left Win";
		case KeyCode_RightWin:		return "Right Win";
		case KeyCode_Apps:			return "Apps";
		case KeyCode_Sleep:			return "Sleep";
		case KeyCode_Numpad0:		return "Numpad 0";
		case KeyCode_Numpad1:		return "Numpad 1";
		case KeyCode_Numpad2:		return "Numpad 2";
		case KeyCode_Numpad3:		return "Numpad 3";
		case KeyCode_Numpad4:		return "Numpad 4";
		case KeyCode_Numpad5:		return "Numpad 5";
		case KeyCode_Numpad6:		return "Numpad 6";
		case KeyCode_Numpad7:		return "Numpad 7";
		case KeyCode_Numpad8:		return "Numpad 8";
		case KeyCode_Numpad9:		return "Numpad 9";
		case KeyCode_Multiply:		return "Multiply";
		case KeyCode_Add:			return "Add";
		case KeyCode_Separator:		return "Separator";
		case KeyCode_Subtract:		return "Subtract";
		case KeyCode_Decimal:		return "Decimal";
		case KeyCode_Divide:		return "Divide";
		case KeyCode_F1:			return "F1";
		case KeyCode_F2:			return "F2";
		case KeyCode_F3:			return "F3";
		case KeyCode_F4:			return "F4";
		case KeyCode_F5:			return "F5";
		case KeyCode_F6:			return "F6";
		case KeyCode_F7:			return "F7";
		case KeyCode_F8:			return "F8";
		case KeyCode_F9:			return "F9";
		case KeyCode_F10:			return "F10";
		case KeyCode_F11:			return "F11";
		case KeyCode_F12:			return "F12";
		case KeyCode_F13:			return "F13";
		case KeyCode_F14:			return "F14";
		case KeyCode_F15:			return "F15";
		case KeyCode_F16:			return "F16";
		case KeyCode_F17:			return "F17";
		case KeyCode_F18:			return "F18";
		case KeyCode_F19:			return "F19";
		case KeyCode_F20:			return "F20";
		case KeyCode_F21:			return "F21";
		case KeyCode_F22:			return "F22";
		case KeyCode_F23:			return "F23";
		case KeyCode_F24:			return "F24";
		case KeyCode_NumLock:		return "Num Lock";
		case KeyCode_Scroll:		return "Scroll";
		case KeyCode_LeftShift:		return "Left Shift";
		case KeyCode_RightShift:	return "Right Shift";
		case KeyCode_LeftControl:	return "Left Control";
		case KeyCode_RightControl:	return "Right Control";
		case KeyCode_LeftAlt:		return "Left Alt";
		case KeyCode_RightAlt:		return "Right Alt";
		}

		assert(false);
		return nullptr;
	}
}

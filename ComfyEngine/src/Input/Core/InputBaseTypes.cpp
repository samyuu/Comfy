#include "InputBaseTypes.h"

namespace Comfy::Input
{
	std::string_view GetKeyCodeNameView(const KeyCode keyCode)
	{
		if (keyCode >= KeyCode_Count)
			return nullptr;

		switch (keyCode)
		{
		case KeyCode_None: return "";
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
		case KeyCode_Ctrl: return "Ctrl";
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
		case KeyCode_Delete: return "Del";
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
		case KeyCode_LeftCtrl: return "Left Ctrl";
		case KeyCode_RightCtrl: return "Right Ctrl";
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
		case KeyCode_OEMPlus: return "+";
		case KeyCode_OEMComma: return ",";
		case KeyCode_OEMMinus: return "-";
		case KeyCode_OEMPeriod: return ".";
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
			return nullptr;
		}
	}

	const char* GetKeyCodeName(const KeyCode keyCode)
	{
		return GetKeyCodeNameView(keyCode).data();
	}

	std::string_view GetButtonNameView(const Button button)
	{
		switch (button)
		{
		case Button::None: return "";
		case Button::DPadUp: return "DPad Up";
		case Button::DPadLeft: return "DPad Left";
		case Button::DPadDown: return "DPad Down";
		case Button::DPadRight: return "DPad Right";
		case Button::FaceUp: return "Face Up";
		case Button::FaceLeft: return "Face Left";
		case Button::FaceDown: return "Face Down";
		case Button::FaceRight: return "Face Right";
		case Button::LeftStickUp: return "Left Stick Up";
		case Button::LeftStickLeft: return "Left Stick Left";
		case Button::LeftStickDown: return "Left Stick Down";
		case Button::LeftStickRight: return "Left Stick Right";
		case Button::LeftStickPush: return "Left Stick Push";
		case Button::RightStickUp: return "Right Stick Up";
		case Button::RightStickLeft: return "Right Stick Left";
		case Button::RightStickDown: return "Right Stick Down";
		case Button::RightStickRight: return "Right Stick Right";
		case Button::RightStickPush: return "Right Stick Push";
		case Button::LeftBumper: return "Left Bumper";
		case Button::RightBumper: return "Right Bumper";
		case Button::LeftTrigger: return "Left Trigger";
		case Button::RightTrigger: return "Right Trigger";
		case Button::Select: return "Select";
		case Button::Start: return "Start";
		case Button::Home: return "Home";
		case Button::TouchPad: return "Touch Pad";

		default:
			return nullptr;
		}
	}

	const char* GetButtonName(const Button button)
	{
		return GetButtonNameView(button).data();
	}

	void ToStringInplace(const KeyCode keyCode, char* buffer, size_t bufferSize)
	{
		const auto keyCodeName = GetKeyCodeNameView(keyCode);
		std::memcpy(buffer, keyCodeName.data(), keyCodeName.size());
		buffer[keyCodeName.size()] = '\0';
	}

	FormatBuffer ToString(const KeyCode keyCode)
	{
		FormatBuffer buffer;
		ToStringInplace(keyCode, buffer.data(), buffer.size());
		return buffer;
	}

	void ToStringInplace(const Binding& binding, char* buffer, size_t bufferSize)
	{
		char* bufferStart = buffer;
		char* bufferEnd = buffer + bufferSize;

		auto appendToBuffer = [&](std::string_view toAppend)
		{
			if (toAppend == "")
				return;

			std::memcpy(buffer, toAppend.data(), toAppend.size());
			buffer[toAppend.size()] = '\0';
			buffer += toAppend.size();
			bufferSize -= toAppend.size() + 1;
		};

		buffer[0] = '\0';
		if (binding.Type == BindingType::Keyboard)
		{
			ForEachKeyCodeInKeyModifiers(binding.Data.Keyboard.Modifiers, [&](KeyCode keyCode)
			{
				appendToBuffer(GetKeyCodeName(keyCode));
				appendToBuffer(" + ");
			});

			appendToBuffer(GetKeyCodeName(binding.Data.Keyboard.Key));
		}
		else if (binding.Type == BindingType::Controller)
		{
			appendToBuffer(GetButtonName(binding.Data.Controller.Button));
		}

		assert(buffer <= bufferEnd);
	}

	FormatBuffer ToString(const Binding& binding)
	{
		FormatBuffer buffer;
		ToStringInplace(binding, buffer.data(), buffer.size());
		return buffer;
	}

	void ToStringInplace(const MultiBinding& binding, char* buffer, size_t bufferSize)
	{
		if (binding.BindingCount > 0)
			ToStringInplace(binding.Bindings[0], buffer, bufferSize);
		else
			buffer[0] = '\0';
	}

	FormatBuffer ToString(const MultiBinding& binding)
	{
		FormatBuffer buffer;
		ToStringInplace(binding, buffer.data(), buffer.size());
		return buffer;
	}
}

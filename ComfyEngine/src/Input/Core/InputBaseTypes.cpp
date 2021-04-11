#include "InputBaseTypes.h"
#include "InputSystem.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"

namespace Comfy::Input
{
	namespace
	{
		struct EnumNamePair
		{
			const char* EnumName;
			const char* DisplayName;
		};

		constexpr std::array<EnumNamePair, KeyCode_Count> KeyCodeNameLookup =
		{
			EnumNamePair { "KeyCode_None", "None" },
			EnumNamePair { "KeyCode_MouseLeft", "Mouse Left" },
			EnumNamePair { "KeyCode_MouseRight", "Mouse Right" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_MouseMiddle", "Mouse Middle" },
			EnumNamePair { "KeyCode_MouseX1", "Mouse X1" },
			EnumNamePair { "KeyCode_MouseX2", "Mouse X2" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Backspace", "Backspace" },
			EnumNamePair { "KeyCode_Tab", "Tab" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Clear", "Clear" },
			EnumNamePair { "KeyCode_Enter", "Enter" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Shift", "Shift" },
			EnumNamePair { "KeyCode_Ctrl", "Ctrl" },
			EnumNamePair { "KeyCode_Alt", "Alt" },
			EnumNamePair { "KeyCode_Pause", "Pause" },
			EnumNamePair { "KeyCode_CapsLock", "Caps Lock" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Escape", "Escape" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Space", "Space" },
			EnumNamePair { "KeyCode_PageUp", "Page Up" },
			EnumNamePair { "KeyCode_PageDown", "Page Down" },
			EnumNamePair { "KeyCode_End", "End" },
			EnumNamePair { "KeyCode_Home", "Home" },
			EnumNamePair { "KeyCode_Left", "Left" },
			EnumNamePair { "KeyCode_Up", "Up" },
			EnumNamePair { "KeyCode_Right", "Right" },
			EnumNamePair { "KeyCode_Down", "Down" },
			EnumNamePair { "KeyCode_Select", "Select" },
			EnumNamePair { "KeyCode_Print", "Print" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Insert", "Insert" },
			EnumNamePair { "KeyCode_Delete", "Del" },
			EnumNamePair { "KeyCode_Help", "Help" },
			EnumNamePair { "KeyCode_0", "0" },
			EnumNamePair { "KeyCode_1", "1" },
			EnumNamePair { "KeyCode_2", "2" },
			EnumNamePair { "KeyCode_3", "3" },
			EnumNamePair { "KeyCode_4", "4" },
			EnumNamePair { "KeyCode_5", "5" },
			EnumNamePair { "KeyCode_6", "6" },
			EnumNamePair { "KeyCode_7", "7" },
			EnumNamePair { "KeyCode_8", "8" },
			EnumNamePair { "KeyCode_9", "9" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_A", "A" },
			EnumNamePair { "KeyCode_B", "B" },
			EnumNamePair { "KeyCode_C", "C" },
			EnumNamePair { "KeyCode_D", "D" },
			EnumNamePair { "KeyCode_E", "E" },
			EnumNamePair { "KeyCode_F", "F" },
			EnumNamePair { "KeyCode_G", "G" },
			EnumNamePair { "KeyCode_H", "H" },
			EnumNamePair { "KeyCode_I", "I" },
			EnumNamePair { "KeyCode_J", "J" },
			EnumNamePair { "KeyCode_K", "K" },
			EnumNamePair { "KeyCode_L", "L" },
			EnumNamePair { "KeyCode_M", "M" },
			EnumNamePair { "KeyCode_N", "N" },
			EnumNamePair { "KeyCode_O", "O" },
			EnumNamePair { "KeyCode_P", "P" },
			EnumNamePair { "KeyCode_Q", "Q" },
			EnumNamePair { "KeyCode_R", "R" },
			EnumNamePair { "KeyCode_S", "S" },
			EnumNamePair { "KeyCode_T", "T" },
			EnumNamePair { "KeyCode_U", "U" },
			EnumNamePair { "KeyCode_V", "V" },
			EnumNamePair { "KeyCode_W", "W" },
			EnumNamePair { "KeyCode_X", "X" },
			EnumNamePair { "KeyCode_Y", "Y" },
			EnumNamePair { "KeyCode_Z", "Z" },
			EnumNamePair { "KeyCode_LeftWin", "Left Win" },
			EnumNamePair { "KeyCode_RightWin", "Right Win" },
			EnumNamePair { "KeyCode_Apps", "Apps" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Sleep", "Sleep" },
			EnumNamePair { "KeyCode_Numpad0", "Numpad 0" },
			EnumNamePair { "KeyCode_Numpad1", "Numpad 1" },
			EnumNamePair { "KeyCode_Numpad2", "Numpad 2" },
			EnumNamePair { "KeyCode_Numpad3", "Numpad 3" },
			EnumNamePair { "KeyCode_Numpad4", "Numpad 4" },
			EnumNamePair { "KeyCode_Numpad5", "Numpad 5" },
			EnumNamePair { "KeyCode_Numpad6", "Numpad 6" },
			EnumNamePair { "KeyCode_Numpad7", "Numpad 7" },
			EnumNamePair { "KeyCode_Numpad8", "Numpad 8" },
			EnumNamePair { "KeyCode_Numpad9", "Numpad 9" },
			EnumNamePair { "KeyCode_Multiply", "Multiply" },
			EnumNamePair { "KeyCode_Add", "Add" },
			EnumNamePair { "KeyCode_Separator", "Separator" },
			EnumNamePair { "KeyCode_Subtract", "Subtract" },
			EnumNamePair { "KeyCode_Decimal", "Decimal" },
			EnumNamePair { "KeyCode_Divide", "Divide" },
			EnumNamePair { "KeyCode_F1", "F1" },
			EnumNamePair { "KeyCode_F2", "F2" },
			EnumNamePair { "KeyCode_F3", "F3" },
			EnumNamePair { "KeyCode_F4", "F4" },
			EnumNamePair { "KeyCode_F5", "F5" },
			EnumNamePair { "KeyCode_F6", "F6" },
			EnumNamePair { "KeyCode_F7", "F7" },
			EnumNamePair { "KeyCode_F8", "F8" },
			EnumNamePair { "KeyCode_F9", "F9" },
			EnumNamePair { "KeyCode_F10", "F10" },
			EnumNamePair { "KeyCode_F11", "F11" },
			EnumNamePair { "KeyCode_F12", "F12" },
			EnumNamePair { "KeyCode_F13", "F13" },
			EnumNamePair { "KeyCode_F14", "F14" },
			EnumNamePair { "KeyCode_F15", "F15" },
			EnumNamePair { "KeyCode_F16", "F16" },
			EnumNamePair { "KeyCode_F17", "F17" },
			EnumNamePair { "KeyCode_F18", "F18" },
			EnumNamePair { "KeyCode_F19", "F19" },
			EnumNamePair { "KeyCode_F20", "F20" },
			EnumNamePair { "KeyCode_F21", "F21" },
			EnumNamePair { "KeyCode_F22", "F22" },
			EnumNamePair { "KeyCode_F23", "F23" },
			EnumNamePair { "KeyCode_F24", "F24" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_NumLock", "Num Lock" },
			EnumNamePair { "KeyCode_Scroll", "Scroll" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_LeftShift", "Left Shift" },
			EnumNamePair { "KeyCode_RightShift", "Right Shift" },
			EnumNamePair { "KeyCode_LeftCtrl", "Left Ctrl" },
			EnumNamePair { "KeyCode_RightCtrl", "Right Ctrl" },
			EnumNamePair { "KeyCode_LeftAlt", "Left Alt" },
			EnumNamePair { "KeyCode_RightAlt", "Right Alt" },
			EnumNamePair { "KeyCode_BrowserBack", "Browser Back" },
			EnumNamePair { "KeyCode_BrowserForward",	"Browser Forward" },
			EnumNamePair { "KeyCode_BrowserRefresh",	"Browser Refresh" },
			EnumNamePair { "KeyCode_BrowserStop", "Browser Stop" },
			EnumNamePair { "KeyCode_BrowserSearch",	"Browser Search" },
			EnumNamePair { "KeyCode_BrowserFavorites",	"Browser Favorite" },
			EnumNamePair { "KeyCode_BrowserHome", "Browser Home" },
			EnumNamePair { "KeyCode_VolumeMute", "Volume Mute" },
			EnumNamePair { "KeyCode_VolumeDown", "Volume Down" },
			EnumNamePair { "KeyCode_VolumeUp", "Volume Up" },
			EnumNamePair { "KeyCode_MediaNextTrack",	"Media Next Track" },
			EnumNamePair { "KeyCode_MediaPrevTrack",	"Media Prev Track" },
			EnumNamePair { "KeyCode_MediaStop", "Media Stop" },
			EnumNamePair { "KeyCode_MediaPlayPause",	"Media Play Pause" },
			EnumNamePair { "KeyCode_LaunchMail", "Launch Mail" },
			EnumNamePair { "KeyCode_LaunchMediaSelect", "Launch Media Select" },
			EnumNamePair { "KeyCode_LaunchApp1", "Launch App 1" },
			EnumNamePair { "KeyCode_LaunchApp2", "Launch App 2" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_OEM1", "OEM 1" },
			EnumNamePair { "KeyCode_OEMPlus", "+" },
			EnumNamePair { "KeyCode_OEMComma", "," },
			EnumNamePair { "KeyCode_OEMMinus", "-" },
			EnumNamePair { "KeyCode_OEMPeriod", "." },
			EnumNamePair { "KeyCode_OEM2", "OEM 2" },
			EnumNamePair { "KeyCode_OEM3", "OEM 3" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_OEM4", "OEM 4" },
			EnumNamePair { "KeyCode_OEM5", "OEM 5" },
			EnumNamePair { "KeyCode_OEM6", "OEM 6" },
			EnumNamePair { "KeyCode_OEM7", "OEM 7" },
			EnumNamePair { "KeyCode_OEM8", "OEM 8" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_OEM102", "OEM 102" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_IMEProcessKey",	"IME Process Key" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Packet", "Packet" },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { nullptr, nullptr },
			EnumNamePair { "KeyCode_Attn", "Attn" },
			EnumNamePair { "KeyCode_CrSel", "CrSel" },
			EnumNamePair { "KeyCode_ExSel", "ExSel" },
			EnumNamePair { "KeyCode_EraseEOF", "Erase EOF" },
			EnumNamePair { "KeyCode_Play", "Play" },
			EnumNamePair { "KeyCode_Zoom", "Zoom" },
			EnumNamePair { "KeyCode_NoName", "No Name" },
			EnumNamePair { "KeyCode_PA1", "PA1" },
			EnumNamePair { "KeyCode_OEMClear", "OEM Clear" },
		};

		constexpr std::array<EnumNamePair, EnumCount<Button>()> ButtonNameLookup =
		{
			EnumNamePair { "Button_None", "None" },
			EnumNamePair { "Button_DPadUp", "DPad Up" },
			EnumNamePair { "Button_DPadLeft", "DPad Left" },
			EnumNamePair { "Button_DPadDown", "DPad Down" },
			EnumNamePair { "Button_DPadRight", "DPad Right" },
			EnumNamePair { "Button_FaceUp", "Face Up" },
			EnumNamePair { "Button_FaceLeft", "Face Left" },
			EnumNamePair { "Button_FaceDown", "Face Down" },
			EnumNamePair { "Button_FaceRight", "Face Right" },
			EnumNamePair { "Button_LeftStickUp", "Left Stick Up" },
			EnumNamePair { "Button_LeftStickLeft", "Left Stick Left" },
			EnumNamePair { "Button_LeftStickDown", "Left Stick Down" },
			EnumNamePair { "Button_LeftStickRight", "Left Stick Right" },
			EnumNamePair { "Button_LeftStickClick", "Left Stick Click" },
			EnumNamePair { "Button_RightStickUp", "Right Stick Up" },
			EnumNamePair { "Button_RightStickLeft", "Right Stick Left" },
			EnumNamePair { "Button_RightStickDown", "Right Stick Down" },
			EnumNamePair { "Button_RightStickRight", "Right Stick Right" },
			EnumNamePair { "Button_RightStickClick", "Right Stick Click" },
			EnumNamePair { "Button_LeftBumper", "Left Bumper" },
			EnumNamePair { "Button_RightBumper", "Right Bumper" },
			EnumNamePair { "Button_LeftTrigger", "Left Trigger" },
			EnumNamePair { "Button_RightTrigger", "Right Trigger" },
			EnumNamePair { "Button_Select", "Select" },
			EnumNamePair { "Button_Start", "Start" },
			EnumNamePair { "Button_Home", "Home" },
			EnumNamePair { "Button_TouchPad", "Touch Pad" },
		};

		constexpr std::string_view BindingStorageStringKeyboardPrefix = "Keyboard";
		constexpr std::string_view BindingStorageStringCtrlModifier = "Ctrl+";
		constexpr std::string_view BindingStorageStringCtrlModifierSpace = "Ctrl +";
		constexpr std::string_view BindingStorageStringShiftModifier = "Shift+";
		constexpr std::string_view BindingStorageStringShiftModifierSpace = "Shift +";
		constexpr std::string_view BindingStorageStringAltModifier = "Alt+";
		constexpr std::string_view BindingStorageStringAltModifierSpace = "Alt +";
		constexpr std::string_view BindingStorageStringControllerPrefix = "Controller";
	}

	const char* GetKeyCodeName(const KeyCode keyCode)
	{
		const auto name = IndexOr(keyCode, KeyCodeNameLookup, EnumNamePair {}).DisplayName;
		return (name != nullptr) ? name : "";
	}

	const char* GetKeyCodeEnumName(const KeyCode keyCode)
	{
		const auto name = IndexOr(keyCode, KeyCodeNameLookup, EnumNamePair {}).EnumName;
		return (name != nullptr) ? name : "";
	}

	// HACK: A linear search for all of these isn't exactly the most performant but works fine enough for now...
	KeyCode ParseKeyCodeName(std::string_view keyCodeName)
	{
		for (size_t i = 0; i < KeyCodeNameLookup.size(); i++)
		{
			if (KeyCodeNameLookup[i].DisplayName != nullptr && Util::MatchesInsensitive(keyCodeName, KeyCodeNameLookup[i].DisplayName))
				return static_cast<KeyCode>(i);
		}

		return KeyCode_None;
	}

	KeyCode ParseKeyCodeEnumName(std::string_view keyCodeEnumName)
	{
		for (size_t i = 0; i < KeyCodeNameLookup.size(); i++)
		{
			if (KeyCodeNameLookup[i].EnumName != nullptr && Util::MatchesInsensitive(keyCodeEnumName, KeyCodeNameLookup[i].EnumName))
				return static_cast<KeyCode>(i);
		}

		return KeyCode_None;
	}

	const char* GetButtonName(const Button button)
	{
		const auto name = IndexOr(static_cast<size_t>(button), ButtonNameLookup, EnumNamePair {}).DisplayName;
		return (name != nullptr) ? name : "";
	}

	const char* GetButtonEnumName(const Button button)
	{
		const auto name = IndexOr(static_cast<size_t>(button), ButtonNameLookup, EnumNamePair {}).EnumName;
		return (name != nullptr) ? name : "";
	}

	Button ParseButtonName(std::string_view buttonName)
	{
		for (size_t i = 0; i < ButtonNameLookup.size(); i++)
		{
			if (ButtonNameLookup[i].DisplayName != nullptr && Util::MatchesInsensitive(buttonName, ButtonNameLookup[i].DisplayName))
				return static_cast<Button>(i);
		}

		return Button::None;
	}

	Button ParseButtonEnumName(std::string_view buttonEnumName)
	{
		for (size_t i = 0; i < ButtonNameLookup.size(); i++)
		{
			if (ButtonNameLookup[i].EnumName != nullptr && Util::MatchesInsensitive(buttonEnumName, ButtonNameLookup[i].EnumName))
				return static_cast<Button>(i);
		}

		return Button::None;
	}

	void ToStringInplace(const KeyCode keyCode, char* buffer, size_t bufferSize)
	{
		const std::string_view keyCodeName = GetKeyCodeName(keyCode);
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
			ForEachKeyCodeInKeyModifiers(binding.Keyboard.Modifiers, [&](KeyCode keyCode)
			{
				appendToBuffer(GetKeyCodeName(keyCode));
				appendToBuffer(" + ");
			});

			appendToBuffer(GetKeyCodeName(binding.Keyboard.Key));
		}
		else if (binding.Type == BindingType::Controller)
		{
			appendToBuffer(GetButtonName(binding.Controller.Button));
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

	Binding BindingFromStorageString(std::string_view string)
	{
		auto tryParseAndTrimLeft = [](std::string_view& inOutStripped, const std::string_view prefix) -> bool
		{
			if (!Util::StartsWithInsensitive(inOutStripped, prefix)) return false;
			inOutStripped = Util::TrimLeft(inOutStripped.substr(prefix.size())); return true;
		};
		auto tryParseAndTrimRight = [](std::string_view& inOutStripped, const std::string_view suffix) -> bool
		{
			if (!Util::EndsWithInsensitive(inOutStripped, suffix)) return false;
			inOutStripped = Util::TrimRight(inOutStripped.substr(0, inOutStripped.size() - suffix.size())); return true;
		};

		Binding result = {};
		std::string_view stripped = Util::Trim(string);

		if (tryParseAndTrimLeft(stripped, BindingStorageStringKeyboardPrefix))
		{
			result.Type = BindingType::Keyboard;

			if (tryParseAndTrimLeft(stripped, BindingStorageStringCtrlModifier) || tryParseAndTrimLeft(stripped, BindingStorageStringCtrlModifierSpace))
				result.Keyboard.Modifiers |= KeyModifiers_Ctrl;
			if (tryParseAndTrimLeft(stripped, BindingStorageStringShiftModifier) || tryParseAndTrimLeft(stripped, BindingStorageStringShiftModifierSpace))
				result.Keyboard.Modifiers |= KeyModifiers_Shift;
			if (tryParseAndTrimLeft(stripped, BindingStorageStringAltModifier) || tryParseAndTrimLeft(stripped, BindingStorageStringAltModifierSpace))
				result.Keyboard.Modifiers |= KeyModifiers_Alt;

			result.Keyboard.Key = ParseKeyCodeName(stripped);
		}
		else if (tryParseAndTrimLeft(stripped, BindingStorageStringControllerPrefix))
		{
			result.Type = BindingType::Controller;
			result.Controller.Button = ParseButtonName(stripped);
		}

		return result;
	}

	FormatBuffer BindingToStorageString(const Binding& binding)
	{
		FormatBuffer buffer;
		buffer[0] = '\0';

		char* writeHead = buffer.data();
		auto appendToBufferOne = [&writeHead](std::string_view toAppend)
		{
			if (toAppend.empty())
				return;
			std::memcpy(writeHead, toAppend.data(), toAppend.size());
			writeHead[toAppend.size()] = '\0';
			writeHead += toAppend.size();
		};
		auto appendToBufferTwo = [&writeHead, &appendToBufferOne](std::string_view toAppendA, std::string_view toAppendB)
		{
			appendToBufferOne(toAppendA);
			appendToBufferOne(toAppendB);
		};

		if (binding.Type == BindingType::Keyboard)
		{
			appendToBufferTwo(BindingStorageStringKeyboardPrefix, " ");

#if 1
			if (binding.Keyboard.Modifiers & KeyModifiers_Ctrl) appendToBufferTwo(BindingStorageStringCtrlModifierSpace, " ");
			if (binding.Keyboard.Modifiers & KeyModifiers_Shift) appendToBufferTwo(BindingStorageStringShiftModifierSpace, " ");
			if (binding.Keyboard.Modifiers & KeyModifiers_Alt) appendToBufferTwo(BindingStorageStringAltModifierSpace, " ");
#else
			if (binding.Keyboard.Modifiers & KeyModifiers_Ctrl) appendToBufferOne(BindingStorageStringCtrlModifier);
			if (binding.Keyboard.Modifiers & KeyModifiers_Shift) appendToBufferOne(BindingStorageStringShiftModifier);
			if (binding.Keyboard.Modifiers & KeyModifiers_Alt) appendToBufferOne(BindingStorageStringAltModifier);
#endif

			appendToBufferOne(GetKeyCodeName(binding.Keyboard.Key));
		}
		else if (binding.Type == BindingType::Controller)
		{
			appendToBufferTwo(BindingStorageStringControllerPrefix, " ");
			appendToBufferOne(GetButtonName(binding.Controller.Button));
		}

		return buffer;
	}

	StandardControllerLayoutMapping ControllerLayoutMappingFromStorageString(std::string_view string)
	{
		auto forEachCommaSeparated = [](std::string_view string, auto perCommaFunc)
		{
			for (size_t absoulteIndex = 0; absoulteIndex < string.size(); absoulteIndex++)
			{
				const std::string_view remaining = string.substr(absoulteIndex);
				for (size_t relativeIndex = 0; relativeIndex < remaining.size(); relativeIndex++)
				{
					if (remaining[relativeIndex] == ',')
					{
						perCommaFunc(Util::Trim(remaining.substr(0, relativeIndex)));
						absoulteIndex += relativeIndex;
						break;
					}
					else if (relativeIndex + 1 == remaining.size())
					{
						perCommaFunc(Util::Trim(remaining));
						return;
					}
				}
			}
		};

		StandardControllerLayoutMapping result = {};

		const size_t idSeparator = string.find_first_of('|');
		if (idSeparator == std::string_view::npos)
			return result;

		const size_t nameSeparator = string.substr().find_first_of('|', idSeparator + 1);
		if (nameSeparator == std::string_view::npos)
			return result;

		const size_t buttonsSeparator = string.substr().find_first_of('|', nameSeparator + 1);
		if (buttonsSeparator == std::string_view::npos)
			return result;

		const std::string_view idSubString = string.substr(0, idSeparator);
		const std::string_view nameSubString = string.substr(idSeparator + 1, (nameSeparator - idSeparator) - 1);
		const std::string_view buttonsSubString = string.substr(nameSeparator + 1, (buttonsSeparator - nameSeparator) - 1);
		const std::string_view axesSubString = string.substr(buttonsSeparator + 1, std::string_view::npos);

		result.ProductID = ControllerIDFromString(Util::Trim(idSubString));
		result.Name = Util::Trim(nameSubString);

		size_t buttonIndex = static_cast<size_t>(Button::None) + 1;
		forEachCommaSeparated(buttonsSubString, [&](std::string_view intStr)
		{
			if (buttonIndex < EnumCount<Button>())
				result.StandardToNativeButtons[buttonIndex++] = static_cast<NativeButton>(Util::StringParsing::ParseType<i32>(intStr));
		});

		size_t axisIndex = static_cast<size_t>(Axis::None) + 1;
		forEachCommaSeparated(axesSubString, [&](std::string_view intStr)
		{
			if (axisIndex < EnumCount<Axis>())
				result.StandardToNativeAxes[axisIndex++] = static_cast<NativeAxis>(Util::StringParsing::ParseType<i32>(intStr));
		});

		return result;
	}

	std::string ControllerLayoutMappingToStorageString(const StandardControllerLayoutMapping& layoutMapping)
	{
		auto appendI32Str = [](std::string& outBuffer, const i32 inValue)
		{
			char intStrBuffer[34];
			::_itoa_s(inValue, intStrBuffer, 10);
			outBuffer += intStrBuffer;
		};

		std::string buffer;
		buffer.reserve(FormatBufferSize * 2);
		buffer += ControllerIDToString(layoutMapping.ProductID).data();
		buffer += '|';

		if (std::any_of(layoutMapping.Name.begin(), layoutMapping.Name.end(), [](char c) { return c == '|'; }))
		{
			for (const char c : layoutMapping.Name)
				buffer += (c == '|') ? ' ' : c;
		}
		else
		{
			buffer += layoutMapping.Name;
		}

		buffer += '|';

		for (size_t i = static_cast<size_t>(Button::None) + 1; i < EnumCount<Button>(); i++)
		{
			appendI32Str(buffer, static_cast<i32>(layoutMapping.StandardToNativeButtons[i]));
			if (i + 1 < EnumCount<Button>())
				buffer += ',';
		}

		buffer += '|';

		for (size_t i = static_cast<size_t>(Axis::None) + 1; i < EnumCount<Axis>(); i++)
		{
			appendI32Str(buffer, static_cast<i32>(layoutMapping.StandardToNativeAxes[i]));
			if (i + 1 < EnumCount<Axis>())
				buffer += ',';
		}

		return buffer;
	}
}

#if COMFY_DEBUG && 1
namespace Comfy::Input
{
	constexpr bool ConstexprStrCmp(std::string_view a, std::string_view b)
	{
		if (a.data() == nullptr || a.size() != b.size())
			return false;
		for (size_t i = 0; i < a.size(); i++)
			if (a[i] != b[i])
				return false;
		return true;
	}

#define DEBUG_CHECK_KEYCODE_ENUM_STRING(x) static_assert(ConstexprStrCmp(KeyCodeNameLookup[x].EnumName, #x), #x);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_None);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MouseLeft);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MouseRight);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MouseMiddle);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MouseX1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MouseX2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Backspace);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Tab);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Clear);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Enter);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Shift);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Ctrl);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Alt);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Pause);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_CapsLock);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Escape);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Space);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_PageUp);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_PageDown);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_End);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Home);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Left);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Up);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Right);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Down);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Select);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Print);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Insert);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Delete);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Help);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_0);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_3);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_4);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_5);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_6);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_7);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_8);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_9);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_A);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_B);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_C);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_D);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_E);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_G);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_H);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_I);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_J);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_K);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_L);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_M);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_N);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_O);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_P);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Q);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_R);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_S);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_T);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_U);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_V);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_W);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_X);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Y);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Z);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LeftWin);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_RightWin);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Apps);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Sleep);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad0);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad3);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad4);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad5);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad6);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad7);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad8);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Numpad9);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Multiply);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Add);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Separator);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Subtract);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Decimal);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Divide);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F3);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F4);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F5);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F6);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F7);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F8);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F9);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F10);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F11);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F12);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F13);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F14);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F15);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F16);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F17);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F18);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F19);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F20);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F21);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F22);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F23);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_F24);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_NumLock);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Scroll);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LeftShift);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_RightShift);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LeftCtrl);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_RightCtrl);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LeftAlt);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_RightAlt);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserBack);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserForward);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserRefresh);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserStop);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserSearch);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserFavorites);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_BrowserHome);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_VolumeMute);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_VolumeDown);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_VolumeUp);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MediaNextTrack);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MediaPrevTrack);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MediaStop);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_MediaPlayPause);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LaunchMail);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LaunchMediaSelect);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LaunchApp1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_LaunchApp2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEMPlus);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEMComma);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEMMinus);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEMPeriod);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM2);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM3);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM4);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM5);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM6);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM7);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM8);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEM102);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_IMEProcessKey);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Packet);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Attn);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_CrSel);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_ExSel);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_EraseEOF);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Play);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_Zoom);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_NoName);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_PA1);
	DEBUG_CHECK_KEYCODE_ENUM_STRING(KeyCode_OEMClear);
#undef DEBUG_CHECK_KEYCODE_STRING
}
#endif

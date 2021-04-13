#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Input
{
	using KeyCode = u16;
	enum KeyCodeEnum : KeyCode
	{
		KeyCode_None = 0x00,
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
		KeyCode_Ctrl = 0x11,
		KeyCode_Alt = 0x12,
		KeyCode_Pause = 0x13,
		KeyCode_CapsLock = 0x14,
		KeyCode_Escape = 0x1B,
		KeyCode_Space = 0x20,
		KeyCode_PageUp = 0x21,
		KeyCode_PageDown = 0x22,
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
		KeyCode_LeftCtrl = 0xA2,
		KeyCode_RightCtrl = 0xA3,
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
		KeyCode_OEMSemicolon = 0xBA,
		KeyCode_OEMPlus = 0xBB,
		KeyCode_OEMComma = 0xBC,
		KeyCode_OEMMinus = 0xBD,
		KeyCode_OEMPeriod = 0xBE,
		KeyCode_OEMQuestion = 0xBF,
		KeyCode_OEMTilde = 0xC0,
		KeyCode_OEMOpenBrackets = 0xDB,
		KeyCode_OEMPipe = 0xDC,
		KeyCode_OEMCloseBrackets = 0xDD,
		KeyCode_OEMQuotes = 0xDE,
		KeyCode_OEM8 = 0xDF,
		KeyCode_OEMBackslash = 0xE2,
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
		KeyCode_First = KeyCode_MouseLeft,
		KeyCode_Last = KeyCode_OEMClear,
		KeyCode_MouseFirst = KeyCode_MouseLeft,
		KeyCode_MouseLast = KeyCode_MouseX2,
		KeyCode_KeyboardFirst = KeyCode_Backspace,
		KeyCode_KeyboardLast = KeyCode_OEMClear,
	};

	using KeyModifiers = u8;
	enum KeyModifiersEnum : KeyModifiers
	{
		KeyModifiers_None = 0,
		KeyModifiers_Ctrl = 1 << 0,
		KeyModifiers_Shift = 1 << 1,
		KeyModifiers_Alt = 1 << 2,

		KeyModifiers_CtrlShift = KeyModifiers_Ctrl | KeyModifiers_Shift,
		KeyModifiers_CtrlShiftAlt = KeyModifiers_Ctrl | KeyModifiers_Shift | KeyModifiers_Alt,
		KeyModifiers_ShiftAlt = KeyModifiers_Shift | KeyModifiers_Alt,
		KeyModifiers_All = KeyModifiers_CtrlShiftAlt,
	};

	// NOTE: No longer part of the Keyboard Binding itself to avoid needless complexity for the user, because the desired behavior is usually known at compile time
	//		 and because an "Optionally hold Shift + Key" binding can be simulated with Strict behavior by creating multiple same-key different-modifier binding variants
	using ModifierBehavior = u8;
	enum ModifierBehaviorEnum : ModifierBehavior
	{
		// NOTE: Bindings are only triggered if the *exact* key modifiers are held down; no more, no less.
		//		 Intended for most standard commands, to avoid conflicts between multiple same key bindings only differing in their modifiers
		ModifierBehavior_Strict,
		// NOTE: Bindings are triggered if all of the modifiers are held down even if unspecified modifiers are held down too.
		//		 Intended for mostly single key bindings that make use of modifiers to change their behavior (i.e. holding shift/alt to increase/decrease step distance)
		ModifierBehavior_Relaxed,
	};

	template <typename Func>
	constexpr void ForEachKeyCodeInKeyModifiers(const KeyModifiers modifiers, Func func)
	{
		if (modifiers & KeyModifiers_Ctrl) func(KeyCode_Ctrl);
		if (modifiers & KeyModifiers_Shift) func(KeyCode_Shift);
		if (modifiers & KeyModifiers_Alt) func(KeyCode_Alt);
	}

	constexpr KeyModifiers InvertKeyModifiers(const KeyModifiers modifiers)
	{
		return (~modifiers & KeyModifiers_All);
	}

	constexpr KeyModifiers KeyCodeToKeyModifiers(const KeyCode keyCode)
	{
		if (keyCode == KeyCode_Ctrl) return KeyModifiers_Ctrl;
		if (keyCode == KeyCode_Shift) return KeyModifiers_Shift;
		if (keyCode == KeyCode_Alt) return KeyModifiers_Alt;
		return KeyModifiers_None;
	}
}

namespace Comfy::Input
{
	// NOTE: Unknown layout buttons. Explicitly defined to improve readability. 1-indexed because 0 is always a reserved dummy value and it matches the user display names
	enum class NativeButton : u8
	{
		None,
		Button_01,
		Button_02,
		Button_03,
		Button_04,
		Button_05,
		Button_06,
		Button_07,
		Button_08,
		Button_09,
		Button_10,
		Button_11,
		Button_12,
		Button_13,
		Button_14,
		Button_15,
		Button_16,
		Button_17,
		Button_18,
		Button_19,
		Button_20,
		Button_21,
		Button_22,
		Button_23,
		Button_24,
		Button_25,
		Button_26,
		Button_27,
		Button_28,
		Button_29,
		Button_30,
		Button_31,
		Button_32,

		DPad_01_Up,
		DPad_01_Left,
		DPad_01_Down,
		DPad_01_Right,
		DPad_02_Up,
		DPad_02_Left,
		DPad_02_Down,
		DPad_02_Right,
		DPad_03_Up,
		DPad_03_Left,
		DPad_03_Down,
		DPad_03_Right,
		DPad_04_Up,
		DPad_04_Left,
		DPad_04_Down,
		DPad_04_Right,

		Axis_01_Positive,
		Axis_01_Negative,
		Axis_02_Positive,
		Axis_02_Negative,
		Axis_03_Positive,
		Axis_03_Negative,
		Axis_04_Positive,
		Axis_04_Negative,
		Axis_05_Positive,
		Axis_05_Negative,
		Axis_06_Positive,
		Axis_06_Negative,
		Axis_07_Positive,
		Axis_07_Negative,
		Axis_08_Positive,
		Axis_08_Negative,

		Count,
		FirstButton = Button_01,
		LastButton = Button_32,
		FirstDPad = DPad_01_Up,
		LastDPad = DPad_04_Right,
		FirstAxis = Axis_01_Positive,
		LastAxis = Axis_08_Negative,
		FirstAll = FirstButton,
		LastAll = LastAxis,

		PerButtonSubElements = 1,
		ButtonCount = ((LastButton - FirstButton) / PerButtonSubElements) + 1,
		ButtonElementCount = (ButtonCount * PerButtonSubElements),

		PerDPadSubElements = 4,
		DPadCount = ((LastDPad - FirstDPad) / PerDPadSubElements) + 1,
		DPadElementCount = (DPadCount * PerDPadSubElements),

		PerAxisSubElements = 2,
		AxisCount = ((LastAxis - FirstAxis) / PerAxisSubElements) + 1,
		AxisElementCount = (AxisCount * PerAxisSubElements),
	};

	enum class NativeAxis : u8
	{
		None,
		Axis_01,
		Axis_02,
		Axis_03,
		Axis_04,
		Axis_05,
		Axis_06,
		Axis_07,
		Axis_08,

		Count,
		First = Axis_01,
		Last = Axis_08,
	};

	// NOTE: Standard meant to abstract a DualShock / Xbox style controller layout
	enum class Button : u8
	{
		None,
		DPadUp,
		DPadLeft,
		DPadDown,
		DPadRight,
		FaceUp,
		FaceLeft,
		FaceDown,
		FaceRight,
		LeftStickUp,
		LeftStickLeft,
		LeftStickDown,
		LeftStickRight,
		LeftStickClick,
		RightStickUp,
		RightStickLeft,
		RightStickDown,
		RightStickRight,
		RightStickClick,
		LeftBumper,
		RightBumper,
		LeftTrigger,
		RightTrigger,
		Select,
		Start,
		Home,
		TouchPad,

		Count,
		First = DPadUp,
		Last = TouchPad,
	};

	// NOTE: Standard represented as normalized float values
	enum class Axis : u8
	{
		None,
		LeftStickX,
		LeftStickY,
		RightStickX,
		RightStickY,
		LeftTrigger,
		RightTrigger,

		Count,
		First = LeftStickX,
		Last = RightTrigger,
	};

	// NOTE: Vec2 center normalized wrapper around the original axes
	enum class Stick : u8
	{
		None,
		LeftStick,
		RightStick,

		Count,
		First = LeftStick,
		Last = RightStick,
	};

	struct ControllerID
	{
		std::array<u8, 16> GUID;
	};

	constexpr bool IsValidControllerID(const ControllerID& id)
	{
		for (size_t i = 0; i < id.GUID.size(); i++)
			if (id.GUID[i] != 0)
				return true;
		return false;
	}

	struct StandardControllerLayoutMapping
	{
		ControllerID ProductID;
		std::array<NativeButton, EnumCount<Button>()> StandardToNativeButtons;
		std::array<NativeAxis, EnumCount<Axis>()> StandardToNativeAxes;
		// std::array<f32, EnumCount<Axis>()> AxesDeadZones;
		std::string Name;
	};

	constexpr Button FindStandardButtonForNativeButton(const StandardControllerLayoutMapping& layoutMapping, const NativeButton nativeButton)
	{
		for (size_t i = static_cast<size_t>(Button::First); i <= static_cast<size_t>(Button::Last); i++)
		{
			if (layoutMapping.StandardToNativeButtons[i] == nativeButton)
				return static_cast<Button>(i);
		}
		return Button::None;
	}

	struct ControllerInfoView
	{
		ControllerID InstanceID;
		ControllerID ProductID;
		std::string_view InstanceName;
		std::string_view ProductName;
		i32 ButtonCount;
		i32 DPadCount;
		i32 AxisCount;
	};
}

namespace Comfy::Input
{
	constexpr size_t MaxMultiBindingCount = 8;

	enum class BindingType : u8
	{
		None,
		Keyboard,
		Controller,
		// TODO: Special window focus/hover behavior (?)
		// Mouse,

		Count,
		First = Keyboard,
		Last = Controller,
	};

	struct Binding
	{
		BindingType Type;
		union
		{
			struct
			{
				KeyCode Key;
				KeyModifiers Modifiers;
			} Keyboard;
			struct
			{
				Button Button;
			} Controller;
		};

		constexpr Binding()
			: Type(BindingType::None), Keyboard()
		{
		}
		explicit constexpr Binding(KeyCode key, KeyModifiers modifiers = KeyModifiers_None)
			: Type(BindingType::Keyboard), Keyboard({ key, modifiers })
		{
		}
		explicit constexpr Binding(Button button)
			: Type(BindingType::Controller), Controller({ button })
		{
		}
		constexpr bool IsEmpty() const
		{
			return
				(Type == BindingType::Keyboard) ? (Keyboard.Key == KeyCode_None) :
				(Type == BindingType::Controller) ? (Controller.Button == Button::None) : true;
		}
		constexpr bool operator==(const Binding& other) const
		{
			return
				(Type != other.Type) ? false :
				(Type == BindingType::Keyboard) ? (Keyboard.Key == other.Keyboard.Key) && (Keyboard.Modifiers == other.Keyboard.Modifiers) :
				(Type == BindingType::Controller) ? (Controller.Button == other.Controller.Button) : true;
		}
		constexpr bool operator!=(const Binding& other) const
		{
			return !(*this == other);
		}
	};

	struct MultiBinding
	{
		std::array<Binding, MaxMultiBindingCount> Bindings = {};
		u8 BindingCount = 0;

		constexpr MultiBinding() {}
		explicit constexpr MultiBinding(Binding primary) : Bindings({ primary }), BindingCount(1) {}
		explicit constexpr MultiBinding(Binding primary, Binding secondary) : Bindings({ primary, secondary }), BindingCount(2) {}

		constexpr auto begin() const { return Bindings.begin(); }
		constexpr auto end() const { return Bindings.begin() + BindingCount; }
	};
}

namespace Comfy::Input
{
	constexpr size_t FormatBufferSize = 128;
	using FormatBuffer = std::array<char, FormatBufferSize>;

	const char* GetKeyCodeName(const KeyCode keyCode);
	const char* GetKeyCodeEnumName(const KeyCode keyCode);
	KeyCode ParseKeyCodeName(std::string_view keyCodeName);
	KeyCode ParseKeyCodeEnumName(std::string_view keyCodeEnumName);

	const char* GetButtonName(const Button button);
	const char* GetButtonEnumName(const Button button);
	Button ParseButtonName(std::string_view buttonName);
	Button ParseButtonEnumName(std::string_view buttonEnumName);

	void ToStringInplace(const KeyCode keyCode, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const KeyCode keyCode);

	void ToStringInplace(const Binding& binding, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const Binding& binding);

	void ToStringInplace(const MultiBinding& binding, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const MultiBinding& binding);

	Binding BindingFromStorageString(std::string_view string);
	FormatBuffer BindingToStorageString(const Binding& binding);

	StandardControllerLayoutMapping ControllerLayoutMappingFromStorageString(std::string_view string);
	std::string ControllerLayoutMappingToStorageString(const StandardControllerLayoutMapping& layoutMapping);
}

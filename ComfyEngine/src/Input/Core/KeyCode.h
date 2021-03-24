#pragma once
#include "Types.h"
#include "CoreTypes.h"

// HACK: Gotta restructure all of this...
#include "Input/DirectInput/DS4Button.h"

namespace Comfy::Input
{
	using KeyCode = i16;
	enum KeyCodeEnum : KeyCode
	{
		KeyCode_Unknown = -1,
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

	using KeyModifiers = u8;
	enum KeyModifiersEnum : KeyModifiers
	{
		KeyModifiers_None = 0,
		KeyModifiers_Ctrl = 1 << 0,
		KeyModifiers_Shift = 1 << 1,
		KeyModifiers_Alt = 1 << 2,
		// KeyModifiers_Super = 1 << 3,
		// KeyModifiers_Function = 1 << 4,

		KeyModifiers_CtrlShift = KeyModifiers_Ctrl | KeyModifiers_Shift,
		KeyModifiers_CtrlShiftAlt = KeyModifiers_Ctrl | KeyModifiers_Shift | KeyModifiers_Alt,
		KeyModifiers_All = KeyModifiers_CtrlShiftAlt,
	};

	using ModifierBehavior = u8;
	enum ModifierBehaviorEnum : ModifierBehavior
	{
		ModifierBehavior_Strict,
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

	constexpr size_t MaxMultiBindingCount = 8;

	enum class BindingType : u8
	{
		None,
		Keyboard,
		// TODO: Generic DirectInput support and controller GUID whitelist (?)
		Controller,
		// TODO: Special window focus/hover behavior (?)
		// Mouse,
	};

	struct Binding
	{
		BindingType Type;
		KeyCode Key;
		KeyModifiers KeyModifiers;
		ModifierBehavior Behavior;
		DS4Button Button;
	};

	struct MultiBinding
	{
		std::array<Binding, MaxMultiBindingCount> Bindings;
		u8 BindingCount;

		constexpr auto begin() const { return Bindings.begin(); }
		constexpr auto end() const { return Bindings.begin() + BindingCount; }
	};

	constexpr Binding MakeBinding(KeyCode key, KeyModifiers modifiers = KeyModifiers_None, ModifierBehavior behavior = ModifierBehavior_Strict)
	{
		return { Binding { BindingType::Keyboard, key, modifiers, behavior } };
	}

	constexpr MultiBinding MakeMultiBinding(KeyCode key, KeyModifiers modifiers = KeyModifiers_None, ModifierBehavior behavior = ModifierBehavior_Strict)
	{
		return MultiBinding { { Binding { BindingType::Keyboard, key, modifiers, behavior } }, 1 };
	}

	constexpr MultiBinding MakeMultiBinding(KeyCode key, KeyModifiers modifiers, ModifierBehavior behavior, KeyCode secondaryKey, KeyModifiers secondaryModifiers, ModifierBehavior secondaryBehavior)
	{
		return MultiBinding { { Binding { BindingType::Keyboard, key, modifiers, behavior }, Binding { BindingType::Keyboard, secondaryKey, secondaryModifiers, secondaryBehavior } }, 2 };
	}

	constexpr MultiBinding MakeMultiBinding(DS4Button button)
	{
		return MultiBinding { { Binding { BindingType::Controller, 0, 0, 0, button } }, 1 };
	}

	constexpr MultiBinding MakeMultiBinding(KeyCode key, KeyModifiers modifiers, ModifierBehavior behavior, DS4Button secondaryButton)
	{
		return MultiBinding { { Binding { BindingType::Keyboard, key, modifiers, behavior }, Binding { BindingType::Controller, 0, 0, 0, secondaryButton } }, 2 };
	}

	template <size_t Size>
	constexpr MultiBinding MakeMultiBinding(const std::array<Binding, Size>& initializer)
	{
		static_assert(Size < MaxMultiBindingCount);
		MultiBinding result = {};
		for (size_t i = 0; i < Size; i++)
			result.Bindings[i] = initializer[i];
		result.BindingCount = static_cast<u8>(Size);
		return result;
	}

	using FormatBuffer = std::array<char, 64>;

	std::string_view GetKeyCodeNameView(const KeyCode keyCode);
	const char* GetKeyCodeName(const KeyCode keyCode);

	void ToStringInplace(const KeyCode keyCode, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const KeyCode keyCode);

	void ToStringInplace(const Binding& binding, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const Binding& binding);

	void ToStringInplace(const MultiBinding& binding, char* buffer, size_t bufferSize);
	FormatBuffer ToString(const MultiBinding& binding);
}

namespace Comfy::Input
{
	bool AreAllModifiersDown(const KeyModifiers modifiers);
	bool AreAllModifiersUp(const KeyModifiers modifiers);
	bool AreOnlyModifiersDown(const KeyModifiers modifiers);

	bool IsKeyDown(const KeyCode keyCode);
	bool IsKeyPressed(const KeyCode keyCode, bool repeat = true);
	bool IsKeyReleased(const KeyCode keyCode);

	bool IsDown(const Binding& binding);
	bool IsPressed(const Binding& binding, bool repeat = true);
	bool IsReleased(const Binding& binding);

	bool IsAnyDown(const MultiBinding& binding);
	bool IsAnyPressed(const MultiBinding& binding, bool repeat = true);
	bool IsAnyReleased(const MultiBinding& binding);
	bool IsLastReleased(const MultiBinding& binding);
}

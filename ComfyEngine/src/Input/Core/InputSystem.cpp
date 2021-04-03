#include "InputSystem.h"
#include "Misc/UTF8.h"
#include "Core/Logger.h"
#include "Time/Stopwatch.h"

// NOTE: Relaying to ImGui for input consistency and simplicity. Could easily be changed in the future however if so required
#include "ImGui/Gui.h"

#define DIRECTINPUT_VERSION 0x0800
#include "Core/Win32/ComfyWindows.h"
#include <dinput.h>
#include <bitset>

namespace Comfy::Input
{
	/* // NOTE: Approximate process time to run input frame update
	std::array<bool>
	~0.0433ms (Debug)
	~0.0024ms (Release)

	std::bitset
	~0.1462ms (Debug)
	~0.0029ms (Release)
	*/

#if 1
	template<size_t Size>
	using BitArray = std::array<bool, Size>;
#else
	template<size_t Size>
	using BitArray = std::bitset<Size>;
#endif

	struct DirectInputControllerData
	{
		DIDEVICEINSTANCEA InstanceData;
		IDirectInputDevice8A* Interface;
		DIDEVCAPS Capabilities;
		std::string InstancetName;
		std::string ProductName;
		struct PolledDeviceState
		{
			DIJOYSTATE NativeJoy;
			struct SimplifiedState
			{
				bool Buttons[32];
				struct
				{
					vec2 NormalizedDirecton;
					bool TopLeftDownRightAreHeld[4];
				} DPads[4];
				struct
				{
					f32 NormalizedAbsolute;
					f32 NormalizedAroundCenter;
					bool NegativePositiveTriggerAreHeld[2];
				} Axes[8];
			} Simplified;
		} ThisFrameState, LastFrameState;
	};

	struct SimplifiedCombinedState
	{
		BitArray<KeyCode_Count> KeysDown;
		BitArray<KeyCode_Count> KeysPress;
		BitArray<KeyCode_Count> KeysRepeat;
		BitArray<KeyCode_Count> KeysRelease;
		BitArray<EnumCount<Button>()> ButtonsDown;
		BitArray<EnumCount<Button>()> ButtonsPress;
		BitArray<EnumCount<Button>()> ButtonsRepeat;
		BitArray<EnumCount<Button>()> ButtonsRelease;
		std::array<f32, EnumCount<Axis>()> Axes;
		std::array<vec2, EnumCount<Stick>()> Sticks;
	};

	struct SimplifiedCombinedTimingState
	{
		std::array<f32, EnumCount<Button>()> ButtonHoldDurationSeconds;
		std::array<i32, EnumCount<Button>()> ButtonHoldDurationFrames;
		f32 ElapsedTimeSeconds;
	};

	struct GlobalInputSystemState
	{
		IDirectInput8A* DirectInput;
		const DIDATAFORMAT* JoyDataFormat;
		HWND MainWindowHandle;

		std::vector<StandardControllerLayoutMapping> LayoutMappings;
		std::vector<DirectInputControllerData> ConnectedControllers;

		SimplifiedCombinedState ThisFrameState, LastFrameState;
		SimplifiedCombinedTimingState ThisFrameTiming, LastFrameTiming;

		Stopwatch UpdateFrameStopwatch;
		TimeSpan UpdateFrameStopwatchElapsed;
	};

	static_assert((static_cast<size_t>(NativeButton::LastButton) - static_cast<size_t>(NativeButton::FirstButton) + 1) == ARRAYSIZE(decltype(DirectInputControllerData::PolledDeviceState::NativeJoy)::rgbButtons));
	static_assert((static_cast<size_t>(NativeAxis::Last) - static_cast<size_t>(NativeAxis::First) + 1) == ARRAYSIZE(DirectInputControllerData::PolledDeviceState::SimplifiedState::Axes));
	static_assert(ARRAYSIZE(DirectInputControllerData::PolledDeviceState::SimplifiedState::Buttons) == ARRAYSIZE(decltype(DirectInputControllerData::PolledDeviceState::NativeJoy)::rgbButtons));
	static_assert(ARRAYSIZE(DirectInputControllerData::PolledDeviceState::SimplifiedState::DPads) == ARRAYSIZE(decltype(DirectInputControllerData::PolledDeviceState::NativeJoy)::rgdwPOV));

	namespace Detail
	{
		static_assert(sizeof(ControllerID) == sizeof(GUID));
		inline ControllerID WindowGUIDToControllerID(const GUID& guid) { return *reinterpret_cast<const ControllerID*>(&guid); }
		inline GUID ControllerIDToWindowGUID(const ControllerID& id) { return *reinterpret_cast<const GUID*>(&id); }

		constexpr bool IsValidKey(KeyCode key) { return (key > KeyCode_None && key < KeyCode_Count); }
		constexpr bool IsValidButton(Button button) { return (button > Button::None && button < Button::Count); }
		constexpr bool IsValidAxis(Axis axis) { return (axis > Axis::None && axis < Axis::Count); }
		constexpr bool IsValidStick(Stick stick) { return (stick > Stick::None && stick < Stick::Count); }

		inline vec2 AngleToDirectionVector(f32 degrees)
		{
			const f32 radians = glm::radians(degrees);
			return vec2(glm::cos(radians), glm::sin(radians));
		}

		DirectInputControllerData* FindControllerForInstanceID(GlobalInputSystemState& global, const ControllerID& instanceID)
		{
			const auto guidToFind = ControllerIDToWindowGUID(instanceID);
			const auto foundController = FindIfOrNull(global.ConnectedControllers, [&](auto& c) { return (c.InstanceData.guidInstance == guidToFind); });

			// NOTE: In case the product ID got accidentally passed in instead of the instance ID
			if (foundController == nullptr)
				assert(FindIfOrNull(global.ConnectedControllers, [&](auto& c) { return (c.InstanceData.guidProduct == guidToFind); }) == nullptr);

			return foundController;
		}

		bool IsNativeButtonDownForKnownController(const DirectInputControllerData& controllerData, NativeButton nativeButton)
		{
			if (nativeButton >= NativeButton::FirstButton && nativeButton <= NativeButton::LastButton)
			{
				const auto reativeIndex = static_cast<std::underlying_type_t<NativeButton>>(nativeButton) - static_cast<std::underlying_type_t<NativeButton>>(NativeButton::FirstButton);
				return controllerData.ThisFrameState.Simplified.Buttons[static_cast<std::underlying_type_t<NativeButton>>(reativeIndex)];
			}
			else if (nativeButton >= NativeButton::FirstDPad && nativeButton <= NativeButton::LastDPad)
			{
				const auto reativeIndex = static_cast<std::underlying_type_t<NativeButton>>(nativeButton) - static_cast<std::underlying_type_t<NativeButton>>(NativeButton::FirstDPad);
				const auto dpadIndex = (reativeIndex / 4);
				const auto directionIndex = (reativeIndex % 4);

				return controllerData.ThisFrameState.Simplified.DPads[dpadIndex].TopLeftDownRightAreHeld[directionIndex];
			}
			else if (nativeButton >= NativeButton::FirstAxis && nativeButton <= NativeButton::LastAxis)
			{
				const auto reativeIndex = static_cast<std::underlying_type_t<NativeButton>>(nativeButton) - static_cast<std::underlying_type_t<NativeButton>>(NativeButton::FirstAxis);
				const auto axisIndex = (reativeIndex / 3);
				const auto directionIndex = (reativeIndex % 3);

				return controllerData.ThisFrameState.Simplified.Axes[axisIndex].NegativePositiveTriggerAreHeld[directionIndex];
			}

			return false;
		}

		f32 GetNativeAxisForKnownController(const DirectInputControllerData& controllerData, NativeAxis nativeAxis, bool normalizedCenter)
		{
			if (nativeAxis >= NativeAxis::First && nativeAxis <= NativeAxis::Last)
			{
				const auto reativeIndex = static_cast<std::underlying_type_t<NativeAxis>>(nativeAxis) - static_cast<std::underlying_type_t<NativeAxis>>(NativeAxis::First);
				const auto& axis = controllerData.ThisFrameState.Simplified.Axes[reativeIndex];
				return normalizedCenter ? axis.NormalizedAroundCenter : axis.NormalizedAbsolute;
			}

			return 0.0f;
		}

		bool AreModifiersDownFirst(GlobalInputSystemState& global, KeyCode keyCode, KeyModifiers modifiers)
		{
			const f32 keyDuration = IsValidKey(keyCode) ? GImGui->IO.KeysDownDuration[keyCode] : 0.0f;
			bool allLonger = true;

			ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode modifierKey) { allLonger &= (GImGui->IO.KeysDownDuration[modifierKey] >= keyDuration); });
			return allLonger;
		}

		bool WereModifiersDownFirst(GlobalInputSystemState& global, KeyCode keyCode, KeyModifiers modifiers)
		{
			const f32 keyDuration = IsValidKey(keyCode) ? GImGui->IO.KeysDownDurationPrev[keyCode] : 0.0f;
			bool allLonger = true;

			ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode modifierKey) { allLonger &= (GImGui->IO.KeysDownDurationPrev[modifierKey] >= keyDuration); });
			return allLonger;
		}
	}
}

namespace Comfy::Input
{
	GlobalInputSystemState Global = {};

	void GlobalSystemInitialize(void* windowHandle)
	{
		if (Global.DirectInput != nullptr)
			return;

		const auto result = ::DirectInput8Create(::GetModuleHandleW(nullptr), DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<void**>(&Global.DirectInput), nullptr);
		Global.JoyDataFormat = std::is_same_v<decltype(DirectInputControllerData::PolledDeviceState::NativeJoy), DIJOYSTATE2> ? &c_dfDIJoystick2 : &c_dfDIJoystick;
		Global.MainWindowHandle = static_cast<HWND>(windowHandle);

		if (FAILED(result))
		{
			Logger::LogErrorLine(__FUNCTION__"(): Failed to initialize DirectInput. Error: %d", result);
		}
		else
		{
			GlobalSystemRefreshDevices();
		}

#if 1 // TEMP: Hardcoded DS4 support for now
		static constexpr std::array knownDualShock4GUIDs =
		{
			// NOTE: First Generation:	{05C4054C-0000-0000-0000-504944564944}
			GUID { 0x05C4054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
			// NOTE: Second Generation:	{09CC054C-0000-0000-0000-504944564944}
			GUID { 0x09CC054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
		};

		for (const auto& ds4GUID : knownDualShock4GUIDs)
		{
			auto& ds4Mapping = Global.LayoutMappings.emplace_back();
			ds4Mapping.ProductID = Detail::WindowGUIDToControllerID(ds4GUID);
			ds4Mapping.Buttons[static_cast<u8>(Button::DPadUp)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstDPad) + 0);
			ds4Mapping.Buttons[static_cast<u8>(Button::DPadLeft)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstDPad) + 1);
			ds4Mapping.Buttons[static_cast<u8>(Button::DPadDown)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstDPad) + 2);
			ds4Mapping.Buttons[static_cast<u8>(Button::DPadRight)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstDPad) + 3);
			ds4Mapping.Buttons[static_cast<u8>(Button::FaceUp)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 3);
			ds4Mapping.Buttons[static_cast<u8>(Button::FaceLeft)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 0);
			ds4Mapping.Buttons[static_cast<u8>(Button::FaceDown)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 1);
			ds4Mapping.Buttons[static_cast<u8>(Button::FaceRight)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 2);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftStickUp)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 3);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftStickLeft)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 0);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftStickDown)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 4);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftStickRight)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 1);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftStickPush)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 10);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightStickUp)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 15);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightStickLeft)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 6);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightStickDown)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 16);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightStickRight)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstAxis) + 7);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightStickPush)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 11);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftBumper)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 4);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightBumper)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 5);
			ds4Mapping.Buttons[static_cast<u8>(Button::LeftTrigger)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 6);
			ds4Mapping.Buttons[static_cast<u8>(Button::RightTrigger)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 7);
			ds4Mapping.Buttons[static_cast<u8>(Button::Select)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 8);
			ds4Mapping.Buttons[static_cast<u8>(Button::Start)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 9);
			ds4Mapping.Buttons[static_cast<u8>(Button::Home)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 12);
			ds4Mapping.Buttons[static_cast<u8>(Button::TouchPad)] = static_cast<NativeButton>(static_cast<u8>(NativeButton::FirstButton) + 13);
			ds4Mapping.Axes[static_cast<u8>(Axis::LeftStickX)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 0);
			ds4Mapping.Axes[static_cast<u8>(Axis::LeftStickY)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 1);
			ds4Mapping.Axes[static_cast<u8>(Axis::RightStickX)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 2);
			ds4Mapping.Axes[static_cast<u8>(Axis::RightStickY)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 5);
			ds4Mapping.Axes[static_cast<u8>(Axis::LeftTrigger)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 3);
			ds4Mapping.Axes[static_cast<u8>(Axis::RightTrigger)] = static_cast<NativeAxis>(static_cast<u8>(NativeAxis::First) + 4);
		}
#endif
	}

	void GlobalSystemDispose(void* windowHandle)
	{
		if (Global.DirectInput != nullptr)
		{
			for (auto& controllerData : Global.ConnectedControllers)
			{
				controllerData.Interface->Unacquire();
				controllerData.Interface->Release();
			}
			Global.ConnectedControllers.clear();

			Global.DirectInput->Release();
			Global.DirectInput = nullptr;
		}

		assert(static_cast<HWND>(windowHandle) == Global.MainWindowHandle);
		Global.MainWindowHandle = nullptr;
	}

	void GlobalSystemUpdateFrame(TimeSpan elapsedTime, bool hasApplicationFocus)
	{
		const f32 elapsedTimeSec = static_cast<f32>(elapsedTime.TotalSeconds());
		if (Global.DirectInput == nullptr)
			return;

		Global.UpdateFrameStopwatch.Restart();

		bool wasAnyConnectionLost = false;
		for (auto& controllerData : Global.ConnectedControllers)
		{
			controllerData.LastFrameState = controllerData.ThisFrameState;
			controllerData.ThisFrameState = {};

			if (!hasApplicationFocus)
				continue;

			HRESULT result = DI_OK;
			result = controllerData.Interface->Poll();
			result = controllerData.Interface->GetDeviceState(sizeof(controllerData.ThisFrameState.NativeJoy), &controllerData.ThisFrameState.NativeJoy);

			if (FAILED(result))
			{
				controllerData.Interface->Unacquire();
				controllerData.Interface->Release();
				controllerData.Interface = nullptr;
				wasAnyConnectionLost = true;
			}
			else
			{
				constexpr f32 heldThreshold = 0.5f;

				for (size_t i = 0; i < std::min<size_t>(std::size(controllerData.ThisFrameState.Simplified.Buttons), controllerData.Capabilities.dwButtons); i++)
					controllerData.ThisFrameState.Simplified.Buttons[i] = controllerData.ThisFrameState.NativeJoy.rgbButtons[i];

				for (size_t i = 0; i < std::min<size_t>(std::size(controllerData.ThisFrameState.Simplified.DPads), controllerData.Capabilities.dwPOVs); i++)
				{
					const auto nativePovValue = controllerData.ThisFrameState.NativeJoy.rgdwPOV[i];
					auto& simplifiedDPad = controllerData.ThisFrameState.Simplified.DPads[i];

					if (nativePovValue != -1 || (LOWORD(nativePovValue) != 0xFFFF))
					{
						// NOTE: From { +0.0 } for Top to { +270.0 } for Left
						const f32 clockwiseAngle = (static_cast<f32>(nativePovValue) / 100.0f);

						// NOTE: { -1.0, -1.0 } for Top-Left and { +1.0, +1.0 } for Bottom-Right
						const vec2 direction = Detail::AngleToDirectionVector(clockwiseAngle - 90.0f);

						simplifiedDPad.NormalizedDirecton = direction;
						simplifiedDPad.TopLeftDownRightAreHeld[0] = (-direction.y > heldThreshold);
						simplifiedDPad.TopLeftDownRightAreHeld[1] = (-direction.x > heldThreshold);
						simplifiedDPad.TopLeftDownRightAreHeld[2] = (+direction.y > heldThreshold);
						simplifiedDPad.TopLeftDownRightAreHeld[3] = (+direction.x > heldThreshold);
					}
					else
					{
						simplifiedDPad = {};
					}
				}

				const auto nativeAxes = std::array
				{
					controllerData.ThisFrameState.NativeJoy.lX, controllerData.ThisFrameState.NativeJoy.lY,
					controllerData.ThisFrameState.NativeJoy.lZ, controllerData.ThisFrameState.NativeJoy.lRx,
					controllerData.ThisFrameState.NativeJoy.lRy, controllerData.ThisFrameState.NativeJoy.lRz,
					controllerData.ThisFrameState.NativeJoy.rglSlider[0], controllerData.ThisFrameState.NativeJoy.rglSlider[1],
				};

				assert(std::size(controllerData.ThisFrameState.Simplified.Axes) == std::size(nativeAxes));
				for (size_t i = 0; i < std::min<size_t>(std::size(controllerData.ThisFrameState.Simplified.Axes), controllerData.Capabilities.dwAxes); i++)
				{
					auto& simplifiedAxes = controllerData.ThisFrameState.Simplified.Axes[i];
					simplifiedAxes.NormalizedAbsolute = static_cast<f32>(nativeAxes[i]) / std::numeric_limits<u16>::max();
					simplifiedAxes.NormalizedAroundCenter = static_cast<f32>(nativeAxes[i]) / std::numeric_limits<u16>::max() * 2.0f - 1.0f;
					simplifiedAxes.NegativePositiveTriggerAreHeld[0] = (glm::abs(simplifiedAxes.NormalizedAroundCenter) > heldThreshold && simplifiedAxes.NormalizedAroundCenter < 0.0f);
					simplifiedAxes.NegativePositiveTriggerAreHeld[1] = (glm::abs(simplifiedAxes.NormalizedAroundCenter) > heldThreshold && simplifiedAxes.NormalizedAroundCenter > 0.0f);
					simplifiedAxes.NegativePositiveTriggerAreHeld[3] = (simplifiedAxes.NormalizedAbsolute > heldThreshold);
				}
			}
		}

		if (wasAnyConnectionLost)
		{
			Global.ConnectedControllers.erase(std::remove_if(Global.ConnectedControllers.begin(), Global.ConnectedControllers.end(),
				[](auto& c) { return (c.Interface == nullptr); }), Global.ConnectedControllers.end());
		}

		Global.LastFrameState = Global.ThisFrameState;
		Global.ThisFrameState = {};

		Global.LastFrameTiming = Global.ThisFrameTiming;
		if (!hasApplicationFocus) Global.ThisFrameTiming = {};
		Global.ThisFrameTiming.ElapsedTimeSeconds = elapsedTimeSec;

		if (hasApplicationFocus)
		{
			for (KeyCode i = 0; i < KeyCode_Count; i++)
				Global.ThisFrameState.KeysDown[i] = GImGui->IO.KeysDown[i];
			for (KeyCode i = 0; i < KeyCode_Count; i++)
				Global.ThisFrameState.KeysPress[i] = Gui::IsKeyPressed(i, false);
			for (KeyCode i = 0; i < KeyCode_Count; i++)
				Global.ThisFrameState.KeysRepeat[i] = Gui::IsKeyPressed(i, true);
			for (KeyCode i = 0; i < KeyCode_Count; i++)
				Global.ThisFrameState.KeysRelease[i] = Gui::IsKeyReleased(i);

			for (const auto& controllerData : Global.ConnectedControllers)
			{
				const auto foundMapping = FindIfOrNull(Global.LayoutMappings, [&](const auto& mapping) { return Detail::ControllerIDToWindowGUID(mapping.ProductID) == controllerData.InstanceData.guidProduct; });
				if (foundMapping == nullptr)
					continue;

				for (size_t i = 0; i < EnumCount<Button>(); i++)
				{
					if (Detail::IsNativeButtonDownForKnownController(controllerData, foundMapping->Buttons[i]))
						Global.ThisFrameState.ButtonsDown[i] = true;
				}

				for (size_t i = 0; i < EnumCount<Axis>(); i++)
				{
					const bool isTriggerAxis = (static_cast<Axis>(i) == Axis::LeftTrigger || static_cast<Axis>(i) == Axis::RightTrigger);
					if (auto v = Detail::GetNativeAxisForKnownController(controllerData, foundMapping->Axes[i], !isTriggerAxis); glm::length(v) > glm::length(Global.ThisFrameState.Axes[i]))
						Global.ThisFrameState.Axes[i] = v;
				}

				auto updateStickWithAxes = [&](Stick stick, Axis x, Axis y)
				{
					Global.ThisFrameState.Sticks[static_cast<u8>(stick)] = vec2(
						Global.ThisFrameState.Axes[static_cast<u8>(x)],
						Global.ThisFrameState.Axes[static_cast<u8>(y)]);
				};

				updateStickWithAxes(Stick::LeftStick, Axis::LeftStickX, Axis::LeftStickY);
				updateStickWithAxes(Stick::RightStick, Axis::RightStickX, Axis::RightStickY);
			}

			for (size_t i = 0; i < EnumCount<Button>(); i++)
			{
				if (Global.ThisFrameState.ButtonsDown[i])
					Global.ThisFrameTiming.ButtonHoldDurationSeconds[i] += elapsedTimeSec;
				else
					Global.ThisFrameTiming.ButtonHoldDurationSeconds[i] = 0.0f;
			}

			for (size_t i = 0; i < EnumCount<Button>(); i++)
			{
				if (Global.ThisFrameState.ButtonsDown[i])
					Global.ThisFrameTiming.ButtonHoldDurationFrames[i] += 1;
				else
					Global.ThisFrameTiming.ButtonHoldDurationFrames[i] = 0;
			}

			for (size_t i = 0; i < EnumCount<Button>(); i++)
				Global.ThisFrameState.ButtonsPress[i] = Global.ThisFrameState.ButtonsDown[i] && !Global.LastFrameState.ButtonsDown[i];

			for (size_t i = 0; i < EnumCount<Button>(); i++)
				Global.ThisFrameState.ButtonsRelease[i] = !Global.ThisFrameState.ButtonsDown[i] && Global.LastFrameState.ButtonsDown[i];

			const f32 repeatDelay = GImGui->IO.KeyRepeatDelay;
			const f32 repeatRate = GImGui->IO.KeyRepeatRate;
			if (repeatRate > 0.0f)
			{
				for (size_t i = 0; i < EnumCount<Button>(); i++)
				{
					f32& downDuration = Global.ThisFrameTiming.ButtonHoldDurationSeconds[i];
					const bool downThisFrame = Global.ThisFrameState.ButtonsDown[i];
					const bool downLastFrame = Global.LastFrameState.ButtonsDown[i];

					if (downThisFrame && !downLastFrame)
					{
						Global.ThisFrameState.ButtonsRepeat[i] = true;
					}
					else if (downDuration > repeatDelay)
					{
						const i32 repeatAmount = Gui::CalcTypematicPressedRepeatAmount(downDuration, (downDuration - elapsedTimeSec), repeatDelay, repeatRate);
						Global.ThisFrameState.ButtonsRepeat[i] = (repeatAmount > 0);
					}
				}
			}
		}

		Global.UpdateFrameStopwatchElapsed = Global.UpdateFrameStopwatch.GetElapsed();
	}

	void GlobalSystemRefreshDevices()
	{
		if (Global.DirectInput == nullptr)
			return;

		for (auto& controllerData : Global.ConnectedControllers)
		{
			controllerData.Interface->Unacquire();
			controllerData.Interface->Release();
		}
		Global.ConnectedControllers.clear();

		auto gameControllerCallback = [](LPCDIDEVICEINSTANCEA deviceInstance, LPVOID) -> BOOL
		{
			DirectInputControllerData controllerData = {};
			HRESULT result = DI_OK;

			controllerData.InstanceData = *deviceInstance;
			if (FAILED(result = Global.DirectInput->CreateDevice(deviceInstance->guidProduct, &controllerData.Interface, nullptr)))
				return DIENUM_CONTINUE;

			if (FAILED(result = controllerData.Interface->SetDataFormat(Global.JoyDataFormat)))
				return DIENUM_CONTINUE;

			if (FAILED(result = controllerData.Interface->Acquire()))
				return DIENUM_CONTINUE;

			controllerData.Capabilities.dwSize = sizeof(controllerData.Capabilities);
			if (FAILED(result = controllerData.Interface->GetCapabilities(&controllerData.Capabilities)))
				return DIENUM_CONTINUE;

			controllerData.InstancetName = deviceInstance->tszInstanceName;
			controllerData.ProductName = deviceInstance->tszProductName;
			Global.ConnectedControllers.push_back(controllerData);
			return DIENUM_CONTINUE;
		};

		HRESULT result = Global.DirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, static_cast<LPDIENUMDEVICESCALLBACKA>(gameControllerCallback), nullptr, DIEDFL_ATTACHEDONLY);
	}

	std::vector<StandardControllerLayoutMapping>& GlobalSystemGetLayoutMappings()
	{
		return Global.LayoutMappings;
	}

	size_t GlobalSystemGetConnectedControllerCount()
	{
		return Global.ConnectedControllers.size();
	}

	ControllerInfoView GlobalSystemGetConnectedControllerInfoAt(size_t index)
	{
		ControllerInfoView outInfoView = {};
		if (InBounds(index, Global.ConnectedControllers))
		{
			const auto& controller = Global.ConnectedControllers[index];
			outInfoView.InstanceID = Detail::WindowGUIDToControllerID(controller.InstanceData.guidInstance);
			outInfoView.ProductID = Detail::WindowGUIDToControllerID(controller.InstanceData.guidProduct);
			outInfoView.InstanceName = controller.InstancetName;
			outInfoView.ProductName = controller.ProductName;
		}
		return outInfoView;
	}

	TimeSpan GlobalSystemGetUpdateFrameProcessDuration()
	{
		return Global.UpdateFrameStopwatchElapsed;
	}
}

namespace Comfy::Input
{
	FormatBuffer ControllerIDToString(const ControllerID& id)
	{
		FormatBuffer buffer = {};

		wchar_t wideBuffer[FormatBufferSize];
		const int wideSize = ::StringFromGUID2(Detail::ControllerIDToWindowGUID(id), wideBuffer, FormatBufferSize);

		for (size_t i = 0; i < FormatBufferSize; i++)
		{
			if ((buffer[i] = static_cast<char>(wideBuffer[i])) == '\0')
				break;
		}

		return buffer;
	}

	ControllerID ControllerIDFromString(std::string_view string)
	{
		wchar_t wideBuffer[FormatBufferSize] = {};
		for (size_t i = 0; i < std::min(FormatBufferSize - 1, string.size()); i++)
			wideBuffer[i] = string[i];

		GUID outGUID = {};
		HRESULT result = ::CLSIDFromString(wideBuffer, &outGUID);

		return Detail::WindowGUIDToControllerID(outGUID);
	}

	bool IsNativeButtonDown(const ControllerID& instanceID, NativeButton nativeButton)
	{
		const auto* foundController = Detail::FindControllerForInstanceID(Global, instanceID);
		return (foundController != nullptr) ? Detail::IsNativeButtonDownForKnownController(*foundController, nativeButton) : false;
	}

	f32 GetNativeAxis(const ControllerID& instanceID, NativeAxis nativeAxis)
	{
		const auto* foundController = Detail::FindControllerForInstanceID(Global, instanceID);
		return (foundController != nullptr) ? Detail::GetNativeAxisForKnownController(*foundController, nativeAxis, false) : 0.0f;
	}

	bool AreAllModifiersDown(const KeyModifiers modifiers)
	{
		bool allDown = true;
		ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode keyCode) { allDown &= IsKeyDown(keyCode); });
		return allDown;
	}

	bool WereAllModifiersDown(const KeyModifiers modifiers)
	{
		bool allDown = true;
		ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode keyCode) { allDown &= WasKeyDown(keyCode); });
		return allDown;
	}

	bool AreAllModifiersUp(const KeyModifiers modifiers)
	{
		bool allUp = true;
		ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode keyCode) { allUp &= !IsKeyDown(keyCode); });
		return allUp;
	}

	bool WereAllModifiersUp(const KeyModifiers modifiers)
	{
		bool allUp = true;
		ForEachKeyCodeInKeyModifiers(modifiers, [&](KeyCode keyCode) { allUp &= !WasKeyDown(keyCode); });
		return allUp;
	}

	bool AreOnlyModifiersDown(const KeyModifiers modifiers)
	{
		return (AreAllModifiersDown(modifiers) && AreAllModifiersUp(InvertKeyModifiers(modifiers)));
	}

	bool WereOnlyModifiersDown(const KeyModifiers modifiers)
	{
		return (WereAllModifiersDown(modifiers) && WereAllModifiersUp(InvertKeyModifiers(modifiers)));
	}

	bool IsKeyDown(const KeyCode keyCode)
	{
		if (!Detail::IsValidKey(keyCode))
			return false;

		return Global.ThisFrameState.KeysDown[keyCode];
	}

	bool WasKeyDown(const KeyCode keyCode)
	{
		if (!Detail::IsValidKey(keyCode))
			return false;

		return Global.LastFrameState.KeysDown[keyCode];
	}

	bool IsKeyPressed(const KeyCode keyCode, bool repeat)
	{
		if (!Detail::IsValidKey(keyCode))
			return false;

		if (repeat)
			return Global.ThisFrameState.KeysRepeat[keyCode];
		else
			return Global.ThisFrameState.KeysPress[keyCode];
	}

	bool IsKeyReleased(const KeyCode keyCode)
	{
		if (!Detail::IsValidKey(keyCode))
			return false;

		return (!Global.ThisFrameState.KeysDown[keyCode] && Global.LastFrameState.KeysDown[keyCode]);
	}

	bool IsButtonDown(const Button button)
	{
		if (!Detail::IsValidButton(button))
			return false;

		return Global.ThisFrameState.ButtonsDown[static_cast<u8>(button)];
	}

	bool WasButtonDown(const Button button)
	{
		if (!Detail::IsValidButton(button))
			return false;

		return Global.LastFrameState.ButtonsDown[static_cast<u8>(button)];
	}

	bool IsButtonPressed(const Button button, bool repeat)
	{
		if (!Detail::IsValidButton(button))
			return false;

		const auto index = static_cast<u8>(button);
		if (repeat)
			return Global.ThisFrameState.ButtonsRepeat[index];
		else
			return Global.ThisFrameState.ButtonsPress[index];
	}

	bool IsButtonReleased(const Button button)
	{
		if (!Detail::IsValidButton(button))
			return false;

		const auto index = static_cast<u8>(button);
		return Global.ThisFrameState.ButtonsRelease[index];
	}

	f32 GetAxis(const Axis axis)
	{
		if (!Detail::IsValidAxis(axis))
			return false;

		return Global.ThisFrameState.Axes[static_cast<u8>(axis)];
	}

	vec2 GetStick(const Stick stick)
	{
		if (!Detail::IsValidStick(stick))
			return vec2(0.0f);

		return Global.ThisFrameState.Sticks[static_cast<u8>(stick)];
	}

	bool IsDown(const Binding& binding)
	{
		if (binding.Type == BindingType::Keyboard)
		{
			if (binding.Keyboard.Behavior == ModifierBehavior_Strict)
				return IsKeyDown(binding.Keyboard.Key) && AreOnlyModifiersDown(binding.Keyboard.Modifiers) && Detail::AreModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
			else
				return IsKeyDown(binding.Keyboard.Key) && AreAllModifiersDown(binding.Keyboard.Modifiers) && Detail::AreModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
		}
		else if (binding.Type == BindingType::Controller)
		{
			return IsButtonDown(binding.Controller.Button);
		}
		else
		{
			return false;
		}
	}

	bool WasDown(const Binding& binding)
	{
		if (binding.Type == BindingType::Keyboard)
		{
			if (binding.Keyboard.Behavior == ModifierBehavior_Strict)
				return WasKeyDown(binding.Keyboard.Key) && WereOnlyModifiersDown(binding.Keyboard.Modifiers) && Detail::WereModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
			else
				return WasKeyDown(binding.Keyboard.Key) && WereAllModifiersDown(binding.Keyboard.Modifiers) && Detail::WereModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
		}
		else if (binding.Type == BindingType::Controller)
		{
			return WasButtonDown(binding.Controller.Button);
		}
		else
		{
			return false;
		}
	}

	// BUG: Strict behavior keybinding doesn't correctly report a press after having been canceled by pressing an unbound (inverse) modifier
	bool IsPressed(const Binding& binding, bool repeat)
	{
		if (binding.Type == BindingType::Keyboard)
		{
			// NOTE: Still have to explictily check the modifier hold durations here in case of repeat
			if (binding.Keyboard.Behavior == ModifierBehavior_Strict)
				return IsKeyPressed(binding.Keyboard.Key, repeat) && AreOnlyModifiersDown(binding.Keyboard.Modifiers) && Detail::AreModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
			else
				return IsKeyPressed(binding.Keyboard.Key, repeat) && AreAllModifiersDown(binding.Keyboard.Modifiers) && Detail::AreModifiersDownFirst(Global, binding.Keyboard.Key, binding.Keyboard.Modifiers);
		}
		else if (binding.Type == BindingType::Controller)
		{
			return IsButtonPressed(binding.Controller.Button, repeat);
		}
		else
		{
			return false;
		}
	}

	bool IsReleased(const Binding& binding)
	{
		return !IsDown(binding) && WasDown(binding);
	}

	bool IsAnyDown(const MultiBinding& binding)
	{
		return std::any_of(binding.begin(), binding.end(), [](auto& b) { return IsDown(b); });
	}

	bool IsAnyPressed(const MultiBinding& binding, bool repeat)
	{
		return std::any_of(binding.begin(), binding.end(), [repeat](auto& b) { return IsPressed(b, repeat); });
	}

	bool IsAnyReleased(const MultiBinding& binding)
	{
		return std::any_of(binding.begin(), binding.end(), [](auto& b) { return IsReleased(b); });
	}

	bool IsLastReleased(const MultiBinding& binding)
	{
		const bool allUp = std::all_of(binding.begin(), binding.end(), [](auto& b) { return !IsDown(b); });
		const bool anyReleased = std::any_of(binding.begin(), binding.end(), [](auto& b) { return IsReleased(b); });

		return (allUp && anyReleased);
	}
}

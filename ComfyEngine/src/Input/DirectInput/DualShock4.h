#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Input/Core/IInputDevice.h"
#include "DirectInputDevice.h"
#include "DS4State.h"

namespace Comfy::Input
{
	class DualShock4 : public DirectInputDevice, public IInputDevice
	{
	public:
		DualShock4();
		~DualShock4();

	public:
		bool Initialize();
		bool PollInput() override;

		bool Instance_IsDown(DS4Button button) const;
		bool Instance_IsUp(DS4Button button) const;
		bool Instance_IsTapped(DS4Button button) const;
		bool Instance_IsReleased(DS4Button button) const;
		bool Instance_WasDown(DS4Button button) const;
		bool Instance_WasUp(DS4Button button) const;

		JoystickState GetLeftStick() const;
		JoystickState GetRightStick() const;
		JoystickState GetDpad() const;

	public:
		static bool TryInitializeInstance();
		static inline bool GetInstanceInitialized() { return instance != nullptr; }
		static inline void DeleteInstance() { delete instance; instance = nullptr; }
		static inline DualShock4* GetInstance() { return instance; }

		static inline bool IsDown(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsDown(button) : false; }
		static inline bool IsUp(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsUp(button) : true; }
		static inline bool IsTapped(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsTapped(button) : false; }
		static inline bool IsReleased(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsReleased(button) : false; }
		static inline bool WasDown(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_WasDown(button) : false; }
		static inline bool WasUp(DS4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_WasUp(button) : true; }

	private:
		static DualShock4* instance;

		DS4State lastState;
		DS4State currentState;

		const float triggerThreshold = 0.5f;
		const float joystickThreshold = 0.5f;
		const float dpadThreshold = 0.5f;

	private:
		float NormalizeTrigger(long value) const;
		float NormalizeStick(long value) const;
		JoystickState NormalizeStick(long x, long y) const;

		void UpdateInternalDs4State(DS4State& state);

		bool MatchesDirection(JoystickState joystick, DS4Direction directionEnum, float threshold) const;
		bool GetButtonState(DS4State& state, DS4Button button) const;

		// NOTE: DualShock 4 Wireless Controller Product GUID: {09CC054C-0000-0000-0000-504944564944}
		static constexpr std::array GUID_DS4 =
		{
			// NOTE: First Generation:	{05C4054C-0000-0000-0000-504944564944}
			GUID { 0x05C4054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
			// NOTE: Second Generation:	{09CC054C-0000-0000-0000-504944564944}
			GUID { 0x09CC054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
		};
	};
}

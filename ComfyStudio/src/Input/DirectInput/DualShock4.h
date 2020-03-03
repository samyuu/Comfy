#pragma once
#include "CoreTypes.h"
#include "Input/IInputDevice.h"
#include "DirectInputDevice.h"
#include "Ds4State.h"

namespace Comfy
{
	class DualShock4 : public DirectInputDevice, public IInputDevice
	{
	public:
		DualShock4();
		~DualShock4();

		bool Initialize();
		bool PollInput() override;

		bool Instance_IsDown(Ds4Button button) const;
		bool Instance_IsUp(Ds4Button button) const;
		bool Instance_IsTapped(Ds4Button button) const;
		bool Instance_IsReleased(Ds4Button button) const;
		bool Instance_WasDown(Ds4Button button) const;
		bool Instance_WasUp(Ds4Button button) const;

		Joystick GetLeftStick() const;
		Joystick GetRightStick() const;
		Joystick GetDpad() const;

	public:
		static bool TryInitializeInstance();
		static inline bool GetInstanceInitialized() { return instance != nullptr; };
		static inline void DeleteInstance() { delete instance; instance = nullptr; };
		static inline DualShock4* GetInstance() { return instance; };

		static inline bool IsDown(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsDown(button) : false; }
		static inline bool IsUp(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsUp(button) : true; }
		static inline bool IsTapped(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsTapped(button) : false; }
		static inline bool IsReleased(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_IsReleased(button) : false; }
		static inline bool WasDown(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_WasDown(button) : false; }
		static inline bool WasUp(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->Instance_WasUp(button) : true; }

	private:
		static DualShock4* instance;

		Ds4State lastState;
		Ds4State currentState;

		const float triggerThreshold = 0.5f;
		const float joystickThreshold = 0.5f;
		const float dpadThreshold = 0.5f;

	private:
		float NormalizeTrigger(long value) const;
		float NormalizeStick(long value) const;
		Joystick NormalizeStick(long x, long y) const;

		void UpdateInternalDs4State(Ds4State& state);

		bool MatchesDirection(Joystick joystick, Direction directionEnum, float threshold) const;
		bool GetButtonState(Ds4State& state, Ds4Button button) const;

		// NOTE: DualShock 4 Wireless Controller Product GUID: {09CC054C-0000-0000-0000-504944564944}
		static constexpr std::array GUID_Ds4 =
		{
			// NOTE: First Generation:	{05C4054C-0000-0000-0000-504944564944}
			GUID { 0x05C4054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
			// NOTE: Second Generation:	{09CC054C-0000-0000-0000-504944564944}
			GUID { 0x09CC054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
		};
	};
}

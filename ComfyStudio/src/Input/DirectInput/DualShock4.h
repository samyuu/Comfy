#pragma once
#include "../IInputDevice.h"
#include "DirectInputDevice.h"
#include "Ds4State.h"

// DualShock 4 Wireless Controller Product GUID: {09CC054C-0000-0000-0000-504944564944}
const GUID GUID_Ds4[2] =
{
	// First Generation:  {05C4054C-0000-0000-0000-504944564944}
	{ 0x05C4054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
	// Second Generation: {09CC054C-0000-0000-0000-504944564944}
	{ 0x09CC054C, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } },
};

class DualShock4 : public DirectInputDevice, public IInputDevice
{
public:
	DualShock4();
	~DualShock4();

	static bool TryInitializeInstance();

	bool Initialize();
	bool PollInput() override;

	bool _IsDown(Ds4Button button);
	bool _IsUp(Ds4Button button);
	bool _IsTapped(Ds4Button button);
	bool _IsReleased(Ds4Button button);
	bool _WasDown(Ds4Button button);
	bool _WasUp(Ds4Button button);

	inline Joystick GetLeftStick() { return currentState.LeftStick; };
	inline Joystick GetRightStick() { return currentState.RightStick; };
	inline Joystick GetDpad() { return currentState.Dpad.Stick; };

	static inline bool IsDown(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_IsDown(button) : false; }
	static inline bool IsUp(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_IsUp(button) : true; }
	static inline bool IsTapped(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_IsTapped(button) : false; }
	static inline bool IsReleased(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_IsReleased(button) : false; }
	static inline bool WasDown(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_WasDown(button) : false; }
	static inline bool WasUp(Ds4Button button) { return GetInstanceInitialized() ? GetInstance()->_WasUp(button) : true; }

	static inline bool GetInstanceInitialized() { return instance != nullptr; };
	static inline void DeleteInstance() { delete instance; instance = nullptr; };
	static inline DualShock4* GetInstance() { return instance; };

private:
	static DualShock4* instance;

	Ds4State lastState;
	Ds4State currentState;

	const float triggerThreshold = 0.5f;
	const float joystickThreshold = 0.5f;
	const float dpadThreshold = 0.5f;

	inline float NormalizeTrigger(long value) { return (float)value / USHRT_MAX; };
	inline float NormalizeStick(long value) { return (float)value / USHRT_MAX * 2.0f - 1.0f; };
	inline Joystick NormalizeStick(long x, long y) { return Joystick(NormalizeStick(x), NormalizeStick(y)); };

	void UpdateInternalDs4State(Ds4State &state);

	bool MatchesDirection(Joystick joystick, Direction directionEnum, float threshold);
	bool GetButtonState(Ds4State &state, Ds4Button button);
};

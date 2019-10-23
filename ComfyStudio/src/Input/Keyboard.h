#pragma once
#include "IInputDevice.h"
#include "KeyCode.h"

class Keyboard : public IInputDevice
{
public:
	static bool TryInitializeInstance(struct GLFWwindow* window);

	virtual bool PollInput() override;

	bool _IsDown(KeyCode key);
	bool _IsUp(KeyCode key);
	bool _IsTapped(KeyCode key);
	bool _IsReleased(KeyCode key);
	bool _WasDown(KeyCode key);
	bool _WasUp(KeyCode key);

	static inline bool IsDown(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_IsDown(key) : false; }
	static inline bool IsUp(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_IsUp(key) : true; }
	static inline bool IsTapped(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_IsTapped(key) : false; }
	static inline bool IsReleased(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_IsReleased(key) : false; }
	static inline bool WasDown(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_WasDown(key) : false; }
	static inline bool WasUp(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->_WasUp(key) : true; }

	static inline bool GetInstanceInitialized() { return instance != nullptr; };
	static inline void DeleteInstance() { delete instance; instance = nullptr; };
	static inline Keyboard* GetInstance() { return instance; };

private:
	Keyboard(GLFWwindow* window);
	~Keyboard();

	static Keyboard* instance;
	struct GLFWwindow* window;

	bool lastState[KeyCode_Count];
	bool currentState[KeyCode_Count];
};

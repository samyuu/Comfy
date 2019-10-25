#pragma once
#include "IInputDevice.h"
#include "KeyCode.h"

class Keyboard : public IInputDevice
{
public:
	virtual bool PollInput() override;

	bool Instance_IsDown(KeyCode key) const;
	bool Instance_IsUp(KeyCode key) const;
	bool Instance_IsTapped(KeyCode key) const;
	bool Instance_IsReleased(KeyCode key) const;
	bool Instance_WasDown(KeyCode key) const;
	bool Instance_WasUp(KeyCode key) const;

	static inline bool IsDown(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_IsDown(key) : false; }
	static inline bool IsUp(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_IsUp(key) : true; }
	static inline bool IsTapped(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_IsTapped(key) : false; }
	static inline bool IsReleased(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_IsReleased(key) : false; }
	static inline bool WasDown(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_WasDown(key) : false; }
	static inline bool WasUp(KeyCode key) { return GetInstanceInitialized() ? GetInstance()->Instance_WasUp(key) : true; }

public:
	static bool TryInitializeInstance(struct GLFWwindow* window);
	static inline bool GetInstanceInitialized() { return instance != nullptr; };
	static inline void DeleteInstance() { delete instance; instance = nullptr; };
	static inline Keyboard* GetInstance() { return instance; };

private:
	Keyboard(GLFWwindow* window);
	~Keyboard();

	static Keyboard* instance;
	struct GLFWwindow* window;

	bool lastState[KeyCode_Count] = {};
	bool currentState[KeyCode_Count] = {};
};

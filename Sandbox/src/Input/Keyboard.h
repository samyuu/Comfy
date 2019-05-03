#pragma once
#include "IInputDevice.h"
#include "glfw/glfw3.h"

class GLFWwindow;

typedef int KeyCode;

class Keyboard : public IInputDevice
{
public:
	static bool TryInitializeInstance(GLFWwindow* window);

	virtual bool PollInput() override;

	bool _IsDown(KeyCode key);
	bool _IsUp(KeyCode key);
	bool _IsTapped(KeyCode key);
	bool _IsReleased(KeyCode key);
	bool _WasDown(KeyCode key);
	bool _WasUp(KeyCode key);

	static inline bool IsDown(KeyCode key) { return InstanceInitialized() ? GetInstance()->_IsDown(key) : false; }
	static inline bool IsUp(KeyCode key) { return InstanceInitialized() ? GetInstance()->_IsUp(key) : true; }
	static inline bool IsTapped(KeyCode key) { return InstanceInitialized() ? GetInstance()->_IsTapped(key) : false; }
	static inline bool IsReleased(KeyCode key) { return InstanceInitialized() ? GetInstance()->_IsReleased(key) : false; }
	static inline bool WasDown(KeyCode key) { return InstanceInitialized() ? GetInstance()->_WasDown(key) : false; }
	static inline bool WasUp(KeyCode key) { return InstanceInitialized() ? GetInstance()->_WasUp(key) : true; }

	static inline bool InstanceInitialized() { return instance != nullptr; };
	static inline void DeleteInstance() { delete instance; instance = nullptr; };
	static inline Keyboard* GetInstance() { return instance; };

private:
	Keyboard(GLFWwindow* window);
	~Keyboard();

	static Keyboard* instance;
	GLFWwindow* window;

	bool lastState[GLFW_KEY_LAST];
	bool currentState[GLFW_KEY_LAST];
};

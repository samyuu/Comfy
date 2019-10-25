#include "Keyboard.h"
#include <glfw/glfw3.h>

Keyboard* Keyboard::instance = nullptr;

Keyboard::Keyboard(GLFWwindow* window) : window(window)
{
}

Keyboard::~Keyboard()
{
}

bool Keyboard::PollInput()
{
	for (KeyCode i = KeyCode_Space; i < KeyCode_Count; i++)
	{
		lastState[i] = currentState[i];
		currentState[i] = glfwGetKey(window, i);
	}

	return true;
}

bool Keyboard::Instance_IsDown(KeyCode key) const
{
	return currentState[key];
}

bool Keyboard::Instance_IsUp(KeyCode key) const
{
	return !Instance_IsDown(key);
}

bool Keyboard::Instance_IsTapped(KeyCode key) const
{
	return Instance_IsDown(key) && Instance_WasUp(key);
}

bool Keyboard::Instance_IsReleased(KeyCode key) const
{
	return Instance_IsUp(key) && Instance_WasDown(key);
}

bool Keyboard::Instance_WasDown(KeyCode key) const
{
	return lastState[key];
}

bool Keyboard::Instance_WasUp(KeyCode key) const
{
	return !Instance_WasDown(key);
}

bool Keyboard::TryInitializeInstance(GLFWwindow* window)
{
	if (GetInstanceInitialized())
		return true;

	instance = new Keyboard(window);
	return true;
}

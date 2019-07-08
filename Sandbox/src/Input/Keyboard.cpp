#include "Keyboard.h"

Keyboard* Keyboard::instance;

bool Keyboard::TryInitializeInstance(GLFWwindow* window)
{
	if (GetInstanceInitialized())
		return true;

	instance = new Keyboard(window);
	return true;
}

Keyboard::Keyboard(GLFWwindow* window) : window(window)
{
}

Keyboard::~Keyboard()
{
}

bool Keyboard::PollInput()
{
	for (size_t i = FIRST_KEY; i < KEY_COUNT; i++)
	{
		lastState[i] = currentState[i];
		currentState[i] = glfwGetKey(window, i);
	}

	return true;
}

bool Keyboard::_IsDown(KeyCode key)
{
	return currentState[key];
}

bool Keyboard::_IsUp(KeyCode key)
{
	return !_IsDown(key);
}

bool Keyboard::_IsTapped(KeyCode key)
{
	return _IsDown(key) && _WasUp(key);
}

bool Keyboard::_IsReleased(KeyCode key)
{
	return _IsUp(key) && _WasDown(key);
}

bool Keyboard::_WasDown(KeyCode key)
{
	return lastState[key];
}

bool Keyboard::_WasUp(KeyCode key)
{
	return !_WasDown(key);
}

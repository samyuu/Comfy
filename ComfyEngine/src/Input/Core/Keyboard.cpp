#include "Keyboard.h"
#include "Core/Win32/ComfyWindows.h"

namespace Comfy::Input
{
	Keyboard* Keyboard::instance = nullptr;

	Keyboard::Keyboard()
	{
	}

	Keyboard::~Keyboard()
	{
	}

	bool Keyboard::PollInput()
	{
		for (KeyCode i = 0; i < KeyCode_Count; i++)
		{
			lastState[i] = currentState[i];
			currentState[i] = GetAsyncKeyState(i);
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

	bool Keyboard::TryInitializeInstance()
	{
		if (GetInstanceInitialized())
			return true;

		instance = new Keyboard();
		return true;
	}
}

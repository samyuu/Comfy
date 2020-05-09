#pragma once
#define DIRECTINPUT_VERSION 0x0800
#include "Core/Win32/ComfyWindows.h"
#include <dinput.h>

namespace Comfy::Input
{
	extern IDirectInput8A* IDirectInputInstance;

	HRESULT InitializeDirectInput(const HMODULE module);

	bool DirectInputInitialized();
	void DisposeDirectInput();
}

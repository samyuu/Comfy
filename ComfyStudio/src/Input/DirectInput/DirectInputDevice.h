#pragma once
#include "DirectInput.h"

namespace Comfy
{
	class DirectInputDevice
	{
	protected:
		IDirectInputDevice8A* directInputdevice;

		HRESULT DI_CreateDevice(const GUID& guid);
		HRESULT DI_SetDataFormat(LPCDIDATAFORMAT dataFormat);
		HRESULT DI_SetCooperativeLevel(HWND windowHandle, DWORD flags);
		HRESULT DI_Acquire();
		HRESULT DI_Unacquire();
		HRESULT DI_Release();
		HRESULT DI_Poll();
		HRESULT DI_GetDeviceState(DWORD size, LPVOID data);

		void DI_Dispose();
	};
}

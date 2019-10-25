#include "DirectInputDevice.h"

HRESULT DirectInputDevice::DI_CreateDevice(const GUID& guid)
{
	if (!DirectInputInitialized())
		return DIERR_NOTINITIALIZED;

	const auto result = IDirectInputInstance->CreateDevice(guid, &directInputdevice, NULL);
	return result;
}

HRESULT DirectInputDevice::DI_SetDataFormat(LPCDIDATAFORMAT dataFormat)
{
	const auto result = directInputdevice->SetDataFormat(dataFormat);
	return result;
}

HRESULT DirectInputDevice::DI_SetCooperativeLevel(HWND windowHandle, DWORD flags)
{
	const auto result = directInputdevice->SetCooperativeLevel(windowHandle, flags);
	return result;
}

HRESULT DirectInputDevice::DI_Acquire()
{
	const auto result = directInputdevice->Acquire();
	return result;
}

HRESULT DirectInputDevice::DI_Unacquire()
{
	const auto result = directInputdevice->Unacquire();
	return result;
}

HRESULT DirectInputDevice::DI_Release()
{
	const auto result = directInputdevice->Release();
	return result;
}

HRESULT DirectInputDevice::DI_Poll()
{
	const auto result = directInputdevice->Poll();
	return result;
}

HRESULT DirectInputDevice::DI_GetDeviceState(DWORD size, LPVOID data)
{
	const auto result = directInputdevice->GetDeviceState(size, data);
	return result;
}

void DirectInputDevice::DI_Dispose()
{
	if (directInputdevice == nullptr)
		return;

	HRESULT result;
	result = DI_Unacquire();
	result = DI_Release();
}

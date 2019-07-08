#include "DirectInput.h"
#include "Logger.h"

IDirectInput8 *IDirectInputInstance = nullptr;

HRESULT InitializeDirectInput(HMODULE module)
{
	HRESULT result = DirectInput8Create(module, DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&IDirectInputInstance, nullptr);
	
	if (FAILED(result))
		Logger::LogErrorLine(__FUNCTION__"(): Failed to initialize DirectInput. Error: %d", result);

	return result;
}

bool DirectInputInitialized()
{
	return IDirectInputInstance != nullptr;
}

void DisposeDirectInput()
{
	if (IDirectInputInstance == nullptr)
		return;

	IDirectInputInstance->Release();
	IDirectInputInstance = nullptr;
}

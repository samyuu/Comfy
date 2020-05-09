#include "DirectInput.h"
#include "Core/Logger.h"

namespace Comfy::Input
{
	IDirectInput8A* IDirectInputInstance = nullptr;

	HRESULT InitializeDirectInput(const HMODULE module)
	{
		const auto result = DirectInput8Create(module, DIRECTINPUT_VERSION, IID_IDirectInput8A, reinterpret_cast<VOID**>(&IDirectInputInstance), nullptr);

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
}

#include "Types.h"
#include <objbase.h>
#include <mfapi.h>
#include "Win32LeanWindowsHeader.h"

namespace
{
	thread_local struct Win32ThreadLocalData
	{
		i32 CoInitializeCount = 0;
		HRESULT CoInitializeResult = {};

		i32 MFStartupCount = 0;
		HRESULT MFStartupResult = {};
	} Win32ThreadLocal;

}
HRESULT Win32ThreadLocalCoInitializeOnce()
{
	if (Win32ThreadLocal.CoInitializeCount++ == 0)
		Win32ThreadLocal.CoInitializeResult = ::CoInitialize(nullptr);

	return Win32ThreadLocal.CoInitializeResult;
}

HRESULT Win32ThreadLocalCoUnInitializeIfLast()
{
	if (Win32ThreadLocal.CoInitializeCount <= 0)
	{
		assert(false);
		return E_ABORT;
	}

	if (Win32ThreadLocal.CoInitializeCount-- == 1)
		::CoUninitialize();

	return S_OK;
}

HRESULT Win32ThreadLocalMFStartupOnce()
{
	if (Win32ThreadLocal.MFStartupCount++ == 0)
		Win32ThreadLocal.MFStartupResult = ::MFStartup(MF_VERSION, MFSTARTUP_LITE);

	return Win32ThreadLocal.MFStartupResult;
}

HRESULT Win32ThreadLocalMFShutdownIfLast()
{
	if (Win32ThreadLocal.MFStartupCount <= 0)
	{
		assert(false);
		return E_ABORT;
	}

	if (Win32ThreadLocal.MFStartupCount-- == 1)
		return ::MFShutdown();

	return S_OK;
}

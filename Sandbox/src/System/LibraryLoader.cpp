#include "LibraryLoader.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"
#include <Windows.h>

namespace System
{
	constexpr HMODULE InvalidModuleHandle = NULL;

	LibraryLoader::LibraryLoader(const char* libraryName, bool loadOnInit) : libraryName(libraryName), moduleHandle(InvalidModuleHandle)
	{
		if (loadOnInit)
			Load(nullptr);
	}

	LibraryLoader::~LibraryLoader()
	{
	}

	bool LibraryLoader::Load(const char* directory)
	{
		if (GetLibraryLoaded())
		{
			Logger::LogErrorLine(__FUNCTION__ "(): Library already loaded");
			return false;
		}

		wchar_t previousWorkingDirectory[MAX_PATH];
		if (directory != nullptr)
		{
			::GetCurrentDirectoryW(MAX_PATH, previousWorkingDirectory);
			::SetCurrentDirectoryW(Utf8ToUtf16(directory).c_str());
		}

		moduleHandle = ::LoadLibraryW(Utf8ToUtf16(libraryName).c_str());
		bool success = moduleHandle != InvalidModuleHandle;

		if (directory != nullptr)
		{
			::SetCurrentDirectoryW(previousWorkingDirectory);
		}

		if (!success)
		{
			int loadLibraryError = ::GetLastError();
			Logger::LogErrorLine(__FUNCTION__ "(): Unable to load library %s. Error: %d", libraryName.c_str(), loadLibraryError);
		}

		return success;
	}

	void LibraryLoader::UnLoad()
	{
		if (!GetLibraryLoaded())
			return;

		::FreeLibrary(reinterpret_cast<HMODULE>(moduleHandle));
		moduleHandle = InvalidModuleHandle;
	}

	bool LibraryLoader::GetLibraryLoaded() const
	{
		return moduleHandle != InvalidModuleHandle;
	}

	void* LibraryLoader::GetFunctionAddress(const char* functionName) const
	{
		if (!GetLibraryLoaded())
			return nullptr;

		void* functionAddress = ::GetProcAddress(reinterpret_cast<HMODULE>(moduleHandle), functionName);
		return functionAddress;
	}
}

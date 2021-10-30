#include "LibraryLoader.h"
#include "IO/Directory.h"
#include "Core/Logger.h"
#include "Core/Win32/ComfyWindows.h"
#include "Misc/StringUtil.h"

namespace Comfy::System
{
	constexpr HMODULE InvalidModuleHandle = NULL;

	LibraryLoader::LibraryLoader(std::string_view libraryName, bool loadOnInit) : libraryName(libraryName), moduleHandle(InvalidModuleHandle)
	{
		if (loadOnInit)
			Load("");
	}

	bool LibraryLoader::Load(std::string_view loadDirectory, bool loadDirectoryOnly)
	{
		if (GetLibraryLoaded())
		{
			Logger::LogErrorLine(__FUNCTION__ "(): Library already loaded");
			return false;
		}

		if (loadDirectoryOnly)
		{
			assert(!loadDirectory.empty());
			moduleHandle = ::LoadLibraryW(UTF8::WideArg(IO::Path::Combine(loadDirectory, libraryName)).c_str());
		}
		else if (!loadDirectory.empty())
		{
			const auto previousWorkingDirectory = IO::Directory::GetWorkingDirectory();
			IO::Directory::SetWorkingDirectory(loadDirectory);
			moduleHandle = ::LoadLibraryW(UTF8::WideArg(libraryName).c_str());
			IO::Directory::SetWorkingDirectory(previousWorkingDirectory);
		}
		else
		{
			moduleHandle = ::LoadLibraryW(UTF8::WideArg(libraryName).c_str());
		}

		const bool wasSuccessful = (moduleHandle != InvalidModuleHandle);
		if (!wasSuccessful)
		{
			const int loadLibraryError = ::GetLastError();
			Logger::LogErrorLine(__FUNCTION__ "(): Unable to load library %s. Error: %d", libraryName.c_str(), loadLibraryError);
		}

		return wasSuccessful;
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

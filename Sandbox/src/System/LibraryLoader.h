#pragma once
#include <string>
#include <Windows.h>

namespace System
{
	class LibraryLoader
	{
	public:
		LibraryLoader(const char* libraryName);
		~LibraryLoader();

		bool Load(const char* directory = nullptr);
		void UnLoad();

		bool GetLibraryLoaded() const;
		void* GetFunctionAddress(const char* functionName) const;
		
	private:
		std::string libraryName;
		HMODULE moduleHandle;
	};
}

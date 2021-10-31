#pragma once
#include "Types.h"

namespace Comfy::System
{
	class LibraryLoader
	{
	public:
		LibraryLoader(std::string_view libraryName, bool loadOnInit = false);
		~LibraryLoader() = default;

	public:
		bool Load(std::string_view loadDirectory = "", bool loadDirectoryOnly = false);
		void UnLoad();

		bool GetLibraryLoaded() const;
		void* GetFunctionAddress(const char* functionName) const;
		
		template <typename T>
		T* GetFunctionAddress(const char* functionName) const
		{
			static_assert(std::is_function_v<T>, "T must be a function type");
			return reinterpret_cast<T*>(GetFunctionAddress(functionName));
		}

	private:
		std::string libraryName;
		void* moduleHandle;
	};
}

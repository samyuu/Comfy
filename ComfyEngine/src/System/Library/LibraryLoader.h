#pragma once
#include "CoreTypes.h"

namespace Comfy::System
{
	class LibraryLoader
	{
	public:
		LibraryLoader(const char* libraryName, bool loadOnInit = false);
		~LibraryLoader();

		bool Load(const char* directory = nullptr);
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

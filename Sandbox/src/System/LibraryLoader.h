#pragma once
#include "Core/CoreTypes.h"

namespace System
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
		
		template <class T>
		inline T* GetFunctionAddress(const char* functionName) const
		{
			static_assert(std::is_function<T>::value, "T must be a function type");
			return reinterpret_cast<T*>(GetFunctionAddress(functionName));
		}

	private:
		String libraryName;
		void* moduleHandle;
	};
}

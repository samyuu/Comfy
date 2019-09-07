#pragma once
#include "Core/CoreTypes.h"

namespace FileSystem
{
	class FileReader
	{
	private:
		FileReader() = delete;

		template <class TPath, class TData>
		static bool ReadEntireFile(const TPath& filePath, Vector<TData>* buffer);

	public:
		template <class T>
		static inline bool ReadEntireFile(const String& filePath, Vector<T>* buffer);

		template <class T>
		static inline bool ReadEntireFile(const WideString& filePath, Vector<T>* buffer);

	private:
		static void* CreateFileHandle(const String& filePath, bool read);
		static void* CreateFileHandle(const WideString& filePath, bool read);
		static void CloseFileHandle(void* fileHandle);

		static size_t GetFileSize(void* fileHandle);
		static size_t ReadFile(void* fileHandle, void* outputData, size_t dataSize);
	};

	template<class TPath, class TData>
	inline bool FileReader::ReadEntireFile(const TPath& filePath, Vector<TData>* buffer)
	{
		void* fileHandle = CreateFileHandle(filePath, true);
		bool validHandle = reinterpret_cast<int64_t>(fileHandle) > 0;

		if (validHandle)
		{
			size_t fileSize = GetFileSize(fileHandle);
			buffer->resize(fileSize);
			ReadFile(fileHandle, buffer->data(), buffer->size());
			CloseFileHandle(fileHandle);
		}

		return validHandle;
	}

	template<class T>
	inline bool FileReader::ReadEntireFile(const String& filePath, Vector<T>* buffer)
	{
		return ReadEntireFile<String, T>(filePath, buffer);
	}

	template<class T>
	inline bool FileReader::ReadEntireFile(const WideString& filePath, Vector<T>* buffer)
	{
		return ReadEntireFile<WideString, T>(filePath, buffer);
	}
}
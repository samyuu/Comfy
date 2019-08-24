#pragma once
#include <vector>

namespace FileSystem
{
	class FileReader
	{
	private:
		FileReader() = delete;

		template <class TPath, class TData>
		static inline bool ReadEntireFile(const TPath& filePath, std::vector<TData>* buffer)
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
		};

	public:
		template <class T>
		static inline bool ReadEntireFile(const std::string& filePath, std::vector<T>* buffer)
		{
			return ReadEntireFile<std::string, T>(filePath, buffer);
		}

		template <class T>
		static inline bool ReadEntireFile(const std::wstring& filePath, std::vector<T>* buffer)
		{
			return ReadEntireFile<std::wstring, T>(filePath, buffer);
		}

	private:
		static void* CreateFileHandle(const std::string& filePath, bool read);
		static void* CreateFileHandle(const std::wstring& filePath, bool read);
		static void CloseFileHandle(void* fileHandle);

		static size_t GetFileSize(void* fileHandle);
		static size_t ReadFile(void* fileHandle, void* outputData, size_t dataSize);
	};
}
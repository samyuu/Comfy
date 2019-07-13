#include "FileHelper.h"
#include <filesystem>
#include <shlwapi.h>
#include <assert.h>
#include <fstream>

namespace FileSystem
{
	using FileSystemPath = std::filesystem::path;
	using DirectoryIterator = std::filesystem::directory_iterator;

	bool CreateDirectory(const std::string& filePath)
	{
		return ::CreateDirectoryA(filePath.c_str(), NULL);
	}

	bool CreateDirectory(const std::wstring& filePath)
	{
		return ::CreateDirectoryW(filePath.c_str(), NULL);
	}

	bool IsFile(const std::string& filePath)
	{
		return GetFileExtension(filePath) != "";
	}

	bool IsFile(const std::wstring& filePath)
	{
		return GetFileExtension(filePath) != L"";
	}

	bool IsDirectory(const std::string& directory)
	{
		return !IsFile(directory);
	}

	bool IsDirectory(const std::wstring& directory)
	{
		return !IsFile(directory);
	}

	bool FileExists(const std::string& filePath)
	{
		return PathFileExistsA(filePath.c_str()) && IsFile(filePath);
	}

	bool FileExists(const std::wstring& filePath)
	{
		return PathFileExistsW(filePath.c_str()) && IsFile(filePath);
	}

	bool DirectoryExists(const std::string& directory)
	{
		return PathFileExistsA(directory.c_str()) && IsDirectory(directory);
	}

	bool DirectoryExists(const std::wstring& directory)
	{
		return PathFileExistsW(directory.c_str()) && IsDirectory(directory);
	}

	void OpenInExplorer(const std::string& filePath)
	{
		std::string currentDirectory = GetWorkingDirectory();
		currentDirectory.reserve(currentDirectory.size() + filePath.size() + 2);
		currentDirectory += "/";
		currentDirectory += filePath;

		ShellExecuteA(NULL, "open", currentDirectory.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

	void OpenInExplorer(const std::wstring& filePath)
	{
		std::wstring currentDirectory = GetWorkingDirectoryW();
		currentDirectory.reserve(currentDirectory.size() + filePath.size() + 2);
		currentDirectory += L"/";
		currentDirectory += filePath;

		ShellExecuteW(NULL, L"open", currentDirectory.c_str(), NULL, NULL, SW_SHOWDEFAULT);
	}

	void OpenExplorerProperties(const std::string& filePath)
	{
		SHELLEXECUTEINFOA info = { };

		info.cbSize = sizeof info;
		info.lpFile = filePath.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = "properties";

		ShellExecuteExA(&info);
	}

	void OpenExplorerProperties(const std::wstring& filePath)
	{
		SHELLEXECUTEINFOW info = { };

		info.cbSize = sizeof info;
		info.lpFile = filePath.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = L"properties";

		ShellExecuteExW(&info);
	}

	std::string GetWorkingDirectory()
	{
		char buffer[MAX_PATH];
		GetCurrentDirectory(sizeof(buffer), buffer);

		return std::string(buffer);
	}

	std::wstring GetWorkingDirectoryW()
	{
		wchar_t buffer[MAX_PATH];
		GetCurrentDirectoryW(sizeof(buffer), buffer);

		return std::wstring(buffer);
	}

	std::wstring Utf8ToWideString(const std::string& filePath)
	{
		wchar_t widePath[MAX_PATH];
		int wideBytes = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, filePath.c_str(), -1, widePath, sizeof(widePath));

		return widePath;
	}

	std::string Combine(const std::string& pathA, const std::string & pathB)
	{
		if (pathA.size() > 0 && pathA.back() == '/')
			return pathA.substr(0, pathA.length() - 1) + "/" + pathB;

		return pathA + "/" + pathB;
	}

	std::wstring Combine(const std::wstring& pathA, const std::wstring& pathB)
	{
		if (pathA.size() > 0 && pathA.back() == L'/')
			return pathA.substr(0, pathA.length() - 1) + L"/" + pathB;

		return pathA + L"/" + pathB;
	}

	std::string GetFileName(const std::string& filePath, bool extension)
	{
		FileSystemPath path(filePath);
		return extension ? path.filename().string() : (path.has_stem() ? path.stem().string() : "");
	}

	std::wstring GetFileName(const std::wstring& filePath, bool extension)
	{
		FileSystemPath path(filePath);
		return extension ? path.filename().wstring() : (path.has_stem() ? path.stem().wstring() : L"");
	}

	std::string GetDirectory(const std::string& filePath)
	{
		return filePath.substr(0, filePath.find_last_of("/\\"));
	}

	std::wstring GetDirectory(const std::wstring& filePath)
	{
		return filePath.substr(0, filePath.find_last_of(L"/\\"));
	}

	std::string GetFileExtension(const std::string& filePath)
	{
		FileSystemPath path(filePath);
		return path.extension().string();
	}

	std::wstring GetFileExtension(const std::wstring & filePath)
	{
		FileSystemPath path(filePath);
		return path.extension().wstring();
	}

	std::vector<std::string> GetFiles(const std::string& directory)
	{
		std::vector<std::string> files;
		for (const auto& file : DirectoryIterator(directory))
			files.push_back(file.path().u8string());
		return files;
	}

	std::vector<std::wstring> GetFiles(const std::wstring& directory)
	{
		std::vector<std::wstring> files;
		for (const auto& file : DirectoryIterator(directory))
			files.push_back(file.path().wstring());
		return files;
	}

	static HANDLE CreateFileHandleInternal(const std::string& filePath, bool read)
	{
		return CreateFileA(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static HANDLE CreateFileHandleInternal(const std::wstring& filePath, bool read)
	{
		return CreateFileW(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static bool ReadAllBytesInternal(HANDLE fileHandle, std::vector<uint8_t>* buffer)
	{
		LARGE_INTEGER fileSizeLarge = {};
		GetFileSizeEx(fileHandle, &fileSizeLarge);

		DWORD fileSize = fileSizeLarge.QuadPart;
		buffer->resize(fileSize);

		DWORD bytesRead = {};
		ReadFile(fileHandle, buffer->data(), fileSize, &bytesRead, nullptr);
		int error = GetLastError();

		return !FAILED(error) && (bytesRead == fileSize);
	}

	static bool WriteAllBytesInternal(HANDLE fileHandle, const std::vector<uint8_t>& buffer)
	{
		DWORD bytesToWrite = buffer.size();
		DWORD bytesWritten = {};

		WriteFile(fileHandle, buffer.data(), bytesToWrite, &bytesWritten, nullptr);
		int error = GetLastError();

		return !FAILED(error) && (bytesWritten == bytesToWrite);
	}

	bool ReadAllBytes(const std::string& filePath, std::vector<uint8_t>* buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath.c_str(), true);
		int error = GetLastError();

		bool result = ReadAllBytesInternal(fileHandle, buffer);
		CloseHandle(fileHandle);

		return result;
	}

	bool ReadAllBytes(const std::wstring& filePath, std::vector<uint8_t>* buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath.c_str(), true);
		bool error = GetLastError();

		bool result = ReadAllBytesInternal(fileHandle, buffer);
		CloseHandle(fileHandle);

		return result;
	}

	bool WriteAllBytes(const std::string& filePath, const std::vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath.c_str(), false);
		bool error = GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		CloseHandle(fileHandle);

		return result;
	}

	bool WriteAllBytes(const std::wstring& filePath, const std::vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath.c_str(), false);
		bool error = GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		CloseHandle(fileHandle);

		return result;
	}

	bool ReadAllLines(const std::string& filePath, std::vector<std::string>* buffer)
	{
		std::ifstream file(filePath);
		while (true)
		{
			buffer->emplace_back();
			if (!std::getline(file, buffer->back()))
				break;
		}
		return true;
	}

	bool ReadAllLines(const std::wstring& filePath, std::vector<std::wstring>* buffer)
	{
		std::wfstream file(filePath);
		while (true)
		{
			buffer->emplace_back();
			if (!std::getline(file, buffer->back()))
				break;
		}
		return true;
	}
}
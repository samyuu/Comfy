#include "FileHelper.h"
#include "FileReader.h"
#include "Misc/StringHelper.h"
#include <filesystem>
#include <shlwapi.h>
#include <assert.h>
#include <fstream>
#include <regex>

namespace FileSystem
{
	using FileSystemPath = std::filesystem::path;
	using DirectoryIterator = std::filesystem::directory_iterator;

	bool CreateDirectoryFile(const std::wstring& filePath)
	{
		return ::CreateDirectoryW(filePath.c_str(), NULL);
	}

	bool IsFilePath(const std::string& filePath)
	{
		for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; i--)
		{
			if (filePath[i] == '.')
				return true;
		}
		return false;
	}

	bool IsFilePath(const std::wstring& filePath)
	{
		for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; i--)
		{
			if (filePath[i] == L'.')
				return true;
		}
		return false;
	}

	bool IsDirectoryPath(const std::string& directory)
	{
		return ::PathIsDirectoryA(directory.c_str());
	}

	bool IsDirectoryPath(const std::wstring& directory)
	{
		return ::PathIsDirectoryW(directory.c_str());
	}

	bool IsPathRelative(const std::string& path)
	{
		return ::PathIsRelativeA(path.c_str());
	}

	bool IsPathRelative(const std::wstring& path)
	{
		return ::PathIsRelativeW(path.c_str());
	}

	bool FileExists(const std::string& filePath)
	{
		return ::PathFileExistsA(filePath.c_str()) && IsFilePath(filePath);
	}

	bool FileExists(const std::wstring& filePath)
	{
		return ::PathFileExistsW(filePath.c_str()) && IsFilePath(filePath);
	}

	bool DirectoryExists(const std::string& directory)
	{
		return ::PathFileExistsA(directory.c_str()) && IsDirectoryPath(directory);
	}

	bool DirectoryExists(const std::wstring& directory)
	{
		return ::PathFileExistsW(directory.c_str()) && IsDirectoryPath(directory);
	}

	static std::wstring InternalFilterVectorToString(const std::vector<std::string>& filterVector)
	{
		assert(filterVector.size() % 2 == 0);

		static const std::regex whitespace("  *");

		std::wstring filterString;
		for (size_t i = 0; i + 1 < filterVector.size(); i += 2)
		{
			filterString += Utf8ToUtf16(filterVector[i]);
			filterString += L'\0';
			filterString += Utf8ToUtf16(std::regex_replace(filterVector[i + 1], whitespace, ";"));
			filterString += L'\0';
		}
		filterString += L'\0';

		return filterString;
	}

	bool CreateOpenFileDialog(std::wstring& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
	{
		std::wstring currentDirectory = GetWorkingDirectoryW();
		wchar_t filePathBuffer[MAX_PATH]; filePathBuffer[0] = '\0';

		assert(!filter.empty());
		std::wstring filterString = InternalFilterVectorToString(filter);
		std::wstring directoryString = (directory != nullptr) ? Utf8ToUtf16(directory) : L"";
		std::wstring titleString = (title != nullptr) ? Utf8ToUtf16(title) : L"";

		OPENFILENAMEW openFileName = {};
		openFileName.lStructSize = sizeof(OPENFILENAMEW);
		openFileName.hwndOwner = NULL;
		openFileName.lpstrFilter = filterString.c_str();
		openFileName.lpstrFile = filePathBuffer;
		openFileName.nMaxFile = MAX_PATH;
		openFileName.lpstrInitialDir = (directory != nullptr) ? directoryString.c_str() : nullptr;
		openFileName.lpstrTitle = (title != nullptr) ? titleString.c_str() : nullptr;
		openFileName.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
		openFileName.lpstrDefExt = L"";

		bool fileSelected = ::GetOpenFileNameW(&openFileName);
		if (fileSelected)
			outFilePath = std::wstring(filePathBuffer);

		SetWorkingDirectoryW(currentDirectory);
		return fileSelected;
	}

	bool CreateSaveFileDialog(std::wstring& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
	{
		std::wstring currentDirectory = GetWorkingDirectoryW();
		wchar_t filePathBuffer[MAX_PATH] = {};

		assert(!filter.empty());
		std::wstring filterString = InternalFilterVectorToString(filter);
		std::wstring directoryString = (directory != nullptr) ? Utf8ToUtf16(directory) : L"";
		std::wstring titleString = (title != nullptr) ? Utf8ToUtf16(title) : L"";

		OPENFILENAMEW openFileName = {};
		openFileName.lStructSize = sizeof(OPENFILENAMEW);
		openFileName.hwndOwner = NULL;
		openFileName.lpstrFilter = filterString.c_str();
		openFileName.lpstrFile = filePathBuffer;
		openFileName.nMaxFile = MAX_PATH;
		openFileName.lpstrInitialDir = (directory != nullptr) ? directoryString.c_str() : nullptr;
		openFileName.lpstrTitle = (title != nullptr) ? titleString.c_str() : nullptr;
		openFileName.Flags = OFN_EXPLORER | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;

		bool fileSelected = ::GetSaveFileNameW(&openFileName);
		if (fileSelected)
			outFilePath = std::wstring(filePathBuffer);

		SetWorkingDirectoryW(currentDirectory);
		return fileSelected;
	}

	void OpenWithDefaultProgram(const std::wstring& filePath)
	{
		::ShellExecuteW(NULL, L"open", filePath.c_str(), NULL, NULL, SW_SHOW);
	}

	void OpenInExplorer(const std::wstring& filePath)
	{
		if (IsPathRelative(filePath))
		{
			std::wstring currentDirectory = GetWorkingDirectoryW();
			currentDirectory.reserve(currentDirectory.size() + filePath.size() + 2);
			currentDirectory += L"/";
			currentDirectory += filePath;

			::ShellExecuteW(NULL, L"open", currentDirectory.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		}
		else
		{
			::ShellExecuteW(NULL, L"open", filePath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
		}
	}

	void OpenExplorerProperties(const std::wstring& filePath)
	{
		SHELLEXECUTEINFOW info = { };

		info.cbSize = sizeof info;
		info.lpFile = filePath.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = L"properties";

		::ShellExecuteExW(&info);
	}

	void FuckUpWindowsPath(std::string& path)
	{
		std::replace(path.begin(), path.end(), '/', '\\');
	}

	void FuckUpWindowsPath(std::wstring& path)
	{
		std::replace(path.begin(), path.end(), L'/', L'\\');
	}

	void SanitizePath(std::string& path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
	}

	void SanitizePath(std::wstring& path)
	{
		std::replace(path.begin(), path.end(), L'\\', L'/');
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
		::GetCurrentDirectoryW(sizeof(buffer), buffer);

		return std::wstring(buffer);
	}

	void SetWorkingDirectory(const std::string& value)
	{
		::SetCurrentDirectoryA(value.c_str());
	}

	void SetWorkingDirectoryW(const std::wstring& value)
	{
		::SetCurrentDirectoryW(value.c_str());
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
		FileSystemPath path = std::filesystem::u8path(filePath);
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
		FileSystemPath path = std::filesystem::u8path(filePath);
		return path.extension().string();
	}

	std::wstring GetFileExtension(const std::wstring& filePath)
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
		return ::CreateFileA(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static HANDLE CreateFileHandleInternal(const std::wstring& filePath, bool read)
	{
		return ::CreateFileW(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static bool WriteAllBytesInternal(HANDLE fileHandle, const std::vector<uint8_t>& buffer)
	{
		DWORD bytesToWrite = static_cast<DWORD>(buffer.size());
		DWORD bytesWritten = {};

		::WriteFile(fileHandle, buffer.data(), bytesToWrite, &bytesWritten, nullptr);
		int error = ::GetLastError();

		return !FAILED(error) && (bytesWritten == bytesToWrite);
	}

	bool WriteAllBytes(const std::string& filePath, const std::vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		::CloseHandle(fileHandle);

		return result;
	}

	bool WriteAllBytes(const std::wstring& filePath, const std::vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		::CloseHandle(fileHandle);

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
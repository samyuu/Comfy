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

	bool CreateDirectoryFile(const WideString& filePath)
	{
		return ::CreateDirectoryW(filePath.c_str(), NULL);
	}

	bool IsFilePath(const String& filePath)
	{
		for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; i--)
		{
			if (filePath[i] == '.')
				return true;
		}
		return false;
	}

	bool IsFilePath(const WideString& filePath)
	{
		for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; i--)
		{
			if (filePath[i] == L'.')
				return true;
		}
		return false;
	}

	bool IsDirectoryPath(const String& directory)
	{
		return ::PathIsDirectoryA(directory.c_str());
	}

	bool IsDirectoryPath(const WideString& directory)
	{
		return ::PathIsDirectoryW(directory.c_str());
	}

	bool IsPathRelative(const String& path)
	{
		return ::PathIsRelativeA(path.c_str());
	}

	bool IsPathRelative(const WideString& path)
	{
		return ::PathIsRelativeW(path.c_str());
	}

	bool FileExists(const String& filePath)
	{
		return ::PathFileExistsA(filePath.c_str()) && IsFilePath(filePath);
	}

	bool FileExists(const WideString& filePath)
	{
		return ::PathFileExistsW(filePath.c_str()) && IsFilePath(filePath);
	}

	bool DirectoryExists(const String& directory)
	{
		return ::PathFileExistsA(directory.c_str()) && IsDirectoryPath(directory);
	}

	bool DirectoryExists(const WideString& directory)
	{
		return ::PathFileExistsW(directory.c_str()) && IsDirectoryPath(directory);
	}

	static WideString InternalFilterVectorToString(const Vector<String>& filterVector)
	{
		assert(filterVector.size() % 2 == 0);

		static const std::regex whitespace("  *");

		WideString filterString;
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

	bool CreateOpenFileDialog(WideString& outFilePath, const char* title, const char* directory, const Vector<String>& filter)
	{
		WideString currentDirectory = GetWorkingDirectoryW();
		wchar_t filePathBuffer[MAX_PATH]; filePathBuffer[0] = '\0';

		assert(!filter.empty());
		WideString filterString = InternalFilterVectorToString(filter);
		WideString directoryString = (directory != nullptr) ? Utf8ToUtf16(directory) : L"";
		WideString titleString = (title != nullptr) ? Utf8ToUtf16(title) : L"";

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
			outFilePath = WideString(filePathBuffer);

		SetWorkingDirectoryW(currentDirectory);
		return fileSelected;
	}

	bool CreateSaveFileDialog(WideString& outFilePath, const char* title, const char* directory, const Vector<String>& filter)
	{
		WideString currentDirectory = GetWorkingDirectoryW();
		wchar_t filePathBuffer[MAX_PATH] = {};

		assert(!filter.empty());
		WideString filterString = InternalFilterVectorToString(filter);
		WideString directoryString = (directory != nullptr) ? Utf8ToUtf16(directory) : L"";
		WideString titleString = (title != nullptr) ? Utf8ToUtf16(title) : L"";

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
			outFilePath = WideString(filePathBuffer);

		SetWorkingDirectoryW(currentDirectory);
		return fileSelected;
	}

	void OpenWithDefaultProgram(const WideString& filePath)
	{
		::ShellExecuteW(NULL, L"open", filePath.c_str(), NULL, NULL, SW_SHOW);
	}

	void OpenInExplorer(const WideString& filePath)
	{
		if (IsPathRelative(filePath))
		{
			WideString currentDirectory = GetWorkingDirectoryW();
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

	void OpenExplorerProperties(const WideString& filePath)
	{
		SHELLEXECUTEINFOW info = { };

		info.cbSize = sizeof info;
		info.lpFile = filePath.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = L"properties";

		::ShellExecuteExW(&info);
	}

	void FuckUpWindowsPath(String& path)
	{
		std::replace(path.begin(), path.end(), '/', '\\');
	}

	void FuckUpWindowsPath(WideString& path)
	{
		std::replace(path.begin(), path.end(), L'/', L'\\');
	}

	void SanitizePath(String& path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
	}

	void SanitizePath(WideString& path)
	{
		std::replace(path.begin(), path.end(), L'\\', L'/');
	}

	String GetWorkingDirectory()
	{
		char buffer[MAX_PATH];
		GetCurrentDirectory(sizeof(buffer), buffer);

		return String(buffer);
	}

	WideString GetWorkingDirectoryW()
	{
		wchar_t buffer[MAX_PATH];
		::GetCurrentDirectoryW(sizeof(buffer), buffer);

		return WideString(buffer);
	}

	void SetWorkingDirectory(const String& value)
	{
		::SetCurrentDirectoryA(value.c_str());
	}

	void SetWorkingDirectoryW(const WideString& value)
	{
		::SetCurrentDirectoryW(value.c_str());
	}

	String Combine(const String& pathA, const String & pathB)
	{
		if (pathA.size() > 0 && pathA.back() == '/')
			return pathA.substr(0, pathA.length() - 1) + "/" + pathB;

		return pathA + "/" + pathB;
	}

	WideString Combine(const WideString& pathA, const WideString& pathB)
	{
		if (pathA.size() > 0 && pathA.back() == L'/')
			return pathA.substr(0, pathA.length() - 1) + L"/" + pathB;

		return pathA + L"/" + pathB;
	}

	String GetFileName(const String& filePath, bool extension)
	{
		FileSystemPath path = std::filesystem::u8path(filePath);
		return extension ? path.filename().string() : (path.has_stem() ? path.stem().string() : "");
	}

	WideString GetFileName(const WideString& filePath, bool extension)
	{
		FileSystemPath path(filePath);
		return extension ? path.filename().wstring() : (path.has_stem() ? path.stem().wstring() : L"");
	}

	String GetDirectory(const String& filePath)
	{
		return filePath.substr(0, filePath.find_last_of("/\\"));
	}

	WideString GetDirectory(const WideString& filePath)
	{
		return filePath.substr(0, filePath.find_last_of(L"/\\"));
	}

	String GetFileExtension(const String& filePath)
	{
		FileSystemPath path = std::filesystem::u8path(filePath);
		return path.extension().string();
	}

	WideString GetFileExtension(const WideString& filePath)
	{
		FileSystemPath path(filePath);
		return path.extension().wstring();
	}

	Vector<String> GetFiles(const String& directory)
	{
		Vector<String> files;
		for (const auto& file : DirectoryIterator(directory))
			files.push_back(file.path().u8string());
		return files;
	}

	Vector<WideString> GetFiles(const WideString& directory)
	{
		Vector<WideString> files;
		for (const auto& file : DirectoryIterator(directory))
			files.push_back(file.path().wstring());
		return files;
	}

	static HANDLE CreateFileHandleInternal(const String& filePath, bool read)
	{
		return ::CreateFileA(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static HANDLE CreateFileHandleInternal(const WideString& filePath, bool read)
	{
		return ::CreateFileW(filePath.c_str(), read ? GENERIC_READ : GENERIC_WRITE, NULL, NULL, read ? OPEN_EXISTING : OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	static bool WriteAllBytesInternal(HANDLE fileHandle, const Vector<uint8_t>& buffer)
	{
		DWORD bytesToWrite = static_cast<DWORD>(buffer.size());
		DWORD bytesWritten = {};

		::WriteFile(fileHandle, buffer.data(), bytesToWrite, &bytesWritten, nullptr);
		int error = ::GetLastError();

		return !FAILED(error) && (bytesWritten == bytesToWrite);
	}

	bool WriteAllBytes(const String& filePath, const Vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		::CloseHandle(fileHandle);

		return result;
	}

	bool WriteAllBytes(const WideString& filePath, const Vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer);
		::CloseHandle(fileHandle);

		return result;
	}

	bool ReadAllLines(const String& filePath, Vector<String>* buffer)
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

	bool ReadAllLines(const WideString& filePath, Vector<WideString>* buffer)
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
#include "FileHelper.h"
#include "FileReader.h"
#include "Misc/StringHelper.h"
#include <filesystem>
#include <fstream>
#include <shlwapi.h>
#include <shobjidl.h>
#include <assert.h>

#include "FileHelperInternal.h"

namespace Comfy::IO
{
	namespace
	{
		std::wstring FilterVectorToStringInternal(const std::vector<std::string>& filterVector)
		{
			assert(filterVector.size() % 2 == 0);

			std::wstring filterString;
			for (size_t i = 0; i + 1 < filterVector.size(); i += 2)
			{
				filterString += UTF8::Widen(filterVector[i]);
				filterString += L'\0';

				filterString += UTF8::Widen(filterVector[i + 1]);
				filterString += L'\0';
			}
			filterString += L'\0';

			return filterString;
		}
	}

	bool CreateDirectoryFile(std::string_view filePath)
	{
		return ::CreateDirectoryW(UTF8::WideArg(filePath).c_str(), NULL);
	}

	bool IsFilePath(std::string_view filePath)
	{
		for (int i = static_cast<int>(filePath.size()) - 1; i >= 0; i--)
		{
			auto character = filePath[i];

			if (character == '/' || character == '\\')
				break;
			else if (character == '.')
				return true;
		}
		return false;
	}

	bool IsDirectoryPath(std::string_view directory)
	{
		return !IsFilePath(directory);
	}

	bool IsPathRelative(std::string_view path)
	{
		return ::PathIsRelativeW(UTF8::WideArg(path).c_str());
	}

	bool FileExists(std::string_view filePath)
	{
		auto attributes = ::GetFileAttributesW(UTF8::WideArg(filePath).c_str());
		return (attributes != INVALID_FILE_ATTRIBUTES && !(attributes & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool DirectoryExists(std::string_view direction)
	{
		auto attributes = ::GetFileAttributesW(UTF8::WideArg(direction).c_str());
		return (attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY));
	}

	bool CreateOpenFileDialog(std::string& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
	{
		std::string currentDirectory = GetWorkingDirectory();
		wchar_t filePathBuffer[MAX_PATH]; filePathBuffer[0] = '\0';

		assert(!filter.empty());
		std::wstring filterString = FilterVectorToStringInternal(filter);
		std::wstring directoryString = (directory != nullptr) ? UTF8::Widen(directory) : L"";
		std::wstring titleString = (title != nullptr) ? UTF8::Widen(title) : L"";

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
			outFilePath = UTF8::Narrow(filePathBuffer);

		SetWorkingDirectory(currentDirectory);
		return fileSelected;
	}

	bool CreateSaveFileDialog(std::string& outFilePath, const char* title, const char* directory, const std::vector<std::string>& filter)
	{
		std::string currentDirectory = GetWorkingDirectory();
		wchar_t filePathBuffer[MAX_PATH] = {};

		assert(!filter.empty());
		std::wstring filterString = FilterVectorToStringInternal(filter);
		std::wstring directoryString = (directory != nullptr) ? UTF8::Widen(directory) : L"";
		std::wstring titleString = (title != nullptr) ? UTF8::Widen(title) : L"";

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
			outFilePath = UTF8::Narrow(filePathBuffer);

		SetWorkingDirectory(currentDirectory);
		return fileSelected;
	}

	void OpenWithDefaultProgram(std::string_view filePath)
	{
		::ShellExecuteW(NULL, L"open", UTF8::WideArg(filePath).c_str(), NULL, NULL, SW_SHOW);
	}

	void OpenInExplorer(std::string_view filePath)
	{
		if (IsPathRelative(filePath))
		{
			std::string currentDirectory = GetWorkingDirectory();
			currentDirectory.reserve(currentDirectory.size() + filePath.size() + 2);
			currentDirectory += "/";
			currentDirectory += filePath;
			::ShellExecuteW(NULL, L"open", UTF8::WideArg(currentDirectory).c_str(), NULL, NULL, SW_SHOWDEFAULT);
		}
		else
		{
			::ShellExecuteW(NULL, L"open", UTF8::WideArg(filePath).c_str(), NULL, NULL, SW_SHOWDEFAULT);
		}
	}

	void OpenExplorerProperties(std::string_view filePath)
	{
		auto filePathArg = UTF8::WideArg(filePath);

		SHELLEXECUTEINFOW info = {};
		info.cbSize = sizeof info;
		info.lpFile = filePathArg.c_str();
		info.nShow = SW_SHOW;
		info.fMask = SEE_MASK_INVOKEIDLIST;
		info.lpVerb = L"properties";
		::ShellExecuteExW(&info);
	}

	std::string ResolveFileLink(std::string_view filePath)
	{
#if 0
		const struct RAII_CoInitialize
		{
			RAII_CoInitialize() { ::CoInitialize(NULL); };
			~RAII_CoInitialize() { ::CoUninitialize(); };
		} coInitialize;
#endif

		std::wstring resolvedPath;

		IShellLinkW* shellLink;
		if (SUCCEEDED(::CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_IShellLinkW, (LPVOID*)&shellLink)))
		{
			IPersistFile* persistFile;
			if (SUCCEEDED(shellLink->QueryInterface(IID_IPersistFile, (void**)&persistFile)))
			{
				if (SUCCEEDED(persistFile->Load(UTF8::WideArg(filePath).c_str(), STGM_READ)))
				{
					if (SUCCEEDED(shellLink->Resolve(NULL, 0)))
					{
						WCHAR pathBuffer[MAX_PATH];
						WIN32_FIND_DATAW findData;

						if (SUCCEEDED(shellLink->GetPath(pathBuffer, MAX_PATH, &findData, SLGP_SHORTPATH)))
							resolvedPath = pathBuffer;
					}
				}
				persistFile->Release();
			}
			shellLink->Release();
		}

		return UTF8::Narrow(resolvedPath);
	}

	void FuckUpWindowsPath(std::string& path)
	{
		std::replace(path.begin(), path.end(), '/', '\\');
	}

	void SanitizePath(std::string& path)
	{
		std::replace(path.begin(), path.end(), '\\', '/');
	}

	std::string GetWorkingDirectory()
	{
		wchar_t buffer[MAX_PATH];
		::GetCurrentDirectoryW(MAX_PATH, buffer);

		return UTF8::Narrow(buffer);
	}

	void SetWorkingDirectory(std::string_view path)
	{
		::SetCurrentDirectoryW(UTF8::WideArg(path).c_str());
	}

	std::string Combine(std::string_view pathA, std::string_view pathB)
	{
		if (pathA.size() > 0 && pathA.back() == '/')
			return std::string(pathA.substr(0, pathA.length() - 1)) + '/' + std::string(pathB);

		return std::string(pathA) + '/' + std::string(pathB);
	}

	std::string_view GetFileName(std::string_view filePath, bool extension)
	{
		const auto last = filePath.find_last_of("/\\");
		return (last == std::string::npos) ? filePath.substr(0, 0) : (extension ? filePath.substr(last + 1) : filePath.substr(last + 1, filePath.length() - last - 1 - GetFileExtension(filePath).length()));
	}

	std::string_view GetDirectory(std::string_view filePath)
	{
		return filePath.substr(0, filePath.find_last_of("/\\"));
	}

	std::string_view GetFileExtension(std::string_view filePath)
	{
		return filePath.substr(filePath.find_last_of("."));
	}

	std::vector<std::string> GetFiles(std::string_view directory)
	{
		std::vector<std::string> files;
		for (const auto& file : std::filesystem::directory_iterator(directory))
			files.push_back(file.path().u8string());
		return files;
	}

	bool ReadAllBytes(std::string_view filePath, void* buffer, size_t bufferSize)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, true);
		int error = ::GetLastError();

		if (fileHandle == nullptr)
			return false;

		DWORD fileSize = ::GetFileSize(fileHandle, nullptr);
		DWORD bytesRead;

		DWORD bytesToRead = min(fileSize, static_cast<DWORD>(bufferSize));

		::ReadFile(fileHandle, buffer, bytesToRead, &bytesRead, nullptr);
		CloseFileHandleInternal(fileHandle);

		return (bytesRead == bytesToRead);
	}

	bool WriteAllBytes(std::string_view filePath, const void* buffer, size_t bufferSize)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer, bufferSize);
		CloseFileHandleInternal(fileHandle);

		return result;
	}

	bool WriteAllBytes(std::string_view filePath, const std::vector<uint8_t>& buffer)
	{
		HANDLE fileHandle = CreateFileHandleInternal(filePath, false);
		int error = ::GetLastError();

		bool result = WriteAllBytesInternal(fileHandle, buffer.data(), buffer.size());
		CloseFileHandleInternal(fileHandle);

		return result;
	}

	bool ReadAllLines(std::string_view filePath, std::vector<std::string>* buffer)
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
}

#include "FileHelper.h"
#include <filesystem>
#include <shlwapi.h>

namespace FileSystem
{
	using FileSystemPath = std::filesystem::path;
	using DirectoryIterator = std::filesystem::directory_iterator;
	
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
			files.push_back(file.path().string());
		return files;
	}

	std::vector<std::wstring> GetFiles(const std::wstring& directory)
	{
		std::vector<std::wstring> files;
		for (const auto& file : DirectoryIterator(directory))
			files.push_back(file.path().wstring());
		return files;
	}
}
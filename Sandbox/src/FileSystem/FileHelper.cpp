#include "FileHelper.h"
#include <filesystem>
#include <shlwapi.h>

namespace FileSystem
{
	using FileSystemPath = std::filesystem::path;

	bool FileExists(const std::string& filePath)
	{
		return PathFileExistsA(filePath.c_str()) && GetFileExtension(filePath) != "";
	}

	bool FileExists(const std::wstring& filePath)
	{
		return PathFileExistsW(filePath.c_str()) && GetFileExtension(filePath) != L"";
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
}
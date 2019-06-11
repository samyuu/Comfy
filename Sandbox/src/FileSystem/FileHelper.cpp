#include "FileHelper.h"
#include <experimental/filesystem>
#include <shlwapi.h>

namespace filesystem = std::experimental::filesystem;

bool FileExists(const std::string& filePath)
{
	return GetFileExtension(filePath) == "" ? false : PathFileExistsA(filePath.c_str());
}

bool FileExists(const std::wstring& filePath)
{
	return GetFileExtension(filePath) == L"" ? false : PathFileExistsW(filePath.c_str());
}

std::string GetFileName(const std::string& filePath, bool extension)
{
	filesystem::path path(filePath);
	return extension ? path.filename().string() : (path.has_stem() ? path.stem().string() : "");
}

std::wstring GetFileName(const std::wstring& filePath, bool extension)
{
	filesystem::path path(filePath);
	return extension ? path.filename().wstring() : (path.has_stem() ? path.stem().wstring() : L"");
}

std::string GetFileExtension(const std::string& filePath)
{
	filesystem::path path(filePath);
	return path.extension().string();
}

std::wstring GetFileExtension(const std::wstring & filePath)
{
	filesystem::path path(filePath);
	return path.extension().wstring();
}

#pragma once
#include <string>
#include <vector>

namespace FileSystem
{
	bool IsFile(const std::string& filePath);
	bool IsFile(const std::wstring& filePath);

	bool IsDirectory(const std::string& directory);
	bool IsDirectory(const std::wstring& directory);

	bool FileExists(const std::string& filePath);
	bool FileExists(const std::wstring& filePath);

	bool DirectoryExists(const std::string& directory);
	bool DirectoryExists(const std::wstring& directory);

	std::string GetFileName(const std::string& filePath, bool extension = true);
	std::wstring GetFileName(const std::wstring& filePath, bool extension = true);

	std::string GetFileExtension(const std::string& filePath);
	std::wstring GetFileExtension(const std::wstring& filePath);

	std::vector<std::string> GetFiles(const std::string& directory);
	std::vector<std::wstring> GetFiles(const std::wstring& directory);
}

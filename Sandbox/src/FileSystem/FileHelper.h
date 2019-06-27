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

	void OpenInExplorer(const std::string& filePath);
	void OpenInExplorer(const std::wstring& filePath);
	
	std::string GetWorkingDirectory();
	std::wstring GetWorkingDirectoryW();
	
	std::wstring Utf8ToWideString(const std::string& filePath);

	std::string Combine(const std::string& pathA, const std::string& pathB);
	std::wstring Combine(const std::wstring& pathA, const std::wstring& pathB);

	std::string GetFileName(const std::string& filePath, bool extension = true);
	std::wstring GetFileName(const std::wstring& filePath, bool extension = true);

	std::string GetDirectory(const std::string& filePath);
	std::wstring GetDirectory(const std::wstring& filePath);

	std::string GetFileExtension(const std::string& filePath);
	std::wstring GetFileExtension(const std::wstring& filePath);

	std::vector<std::string> GetFiles(const std::string& directory);
	std::vector<std::wstring> GetFiles(const std::wstring& directory);

	bool ReadAllBytes(const std::string& filePath, std::vector<uint8_t>* buffer);
	bool ReadAllBytes(const std::wstring& filePath, std::vector<uint8_t>* buffer);

	bool WriteAllBytes(const std::string& filePath, const std::vector<uint8_t>& buffer);
	bool WriteAllBytes(const std::wstring& filePath, const std::vector<uint8_t>& buffer);

	bool ReadAllLines(const std::string& filePath, std::vector<std::string>& buffer);
	bool ReadAllLines(const std::wstring& filePath, std::vector<std::wstring>& buffer);
}

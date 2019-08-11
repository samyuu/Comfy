#pragma once
#include <string>
#include <vector>

namespace FileSystem
{
	const std::vector<std::string> AllFilesFilter = { "All Files (*.*)", "*.*" };

	bool CreateDirectory(const std::wstring& filePath);

	bool IsFilePath(const std::string& filePath);
	bool IsFilePath(const std::wstring& filePath);

	bool IsDirectoryPath(const std::string& directory);
	bool IsDirectoryPath(const std::wstring& directory);

	bool IsPathRelative(const std::string& path);
	bool IsPathRelative(const std::wstring& path);

	bool FileExists(const std::string& filePath);
	bool FileExists(const std::wstring& filePath);

	bool DirectoryExists(const std::string& directory);
	bool DirectoryExists(const std::wstring& directory);

	bool CreateOpenFileDialog(std::wstring& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);
	bool CreateSaveFileDialog(std::wstring& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);

	void OpenWithDefaultProgram(const std::wstring& filePath);
	void OpenInExplorer(const std::wstring& filePath);
	void OpenExplorerProperties(const std::wstring& filePath);

	void SanitizePath(std::string& path);
	void SanitizePath(std::wstring& path);

	std::string GetWorkingDirectory();
	std::wstring GetWorkingDirectoryW();
	
	void SetWorkingDirectory(const std::string& value);
	void SetWorkingDirectoryW(const std::wstring& value);

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

	bool ReadAllLines(const std::string& filePath, std::vector<std::string>* buffer);
	bool ReadAllLines(const std::wstring& filePath, std::vector<std::wstring>* buffer);
}

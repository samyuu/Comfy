#pragma once
#include "FileReader.h"

namespace Comfy::FileSystem
{
	const std::vector<std::string> AllFilesFilter = { "All Files (*.*)", "*.*" };

	bool CreateDirectoryFile(std::string_view filePath);

	bool IsFilePath(std::string_view filePath);
	bool IsDirectoryPath(std::string_view directory);

	bool IsPathRelative(std::string_view path);
	
	bool FileExists(std::string_view filePath);
	bool DirectoryExists(std::string_view direction);

	bool CreateOpenFileDialog(std::string& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);
	bool CreateSaveFileDialog(std::string& outFilePath, const char* title = nullptr, const char* directory = nullptr, const std::vector<std::string>& filter = AllFilesFilter);

	void OpenWithDefaultProgram(std::string_view filePath);
	void OpenInExplorer(std::string_view filePath);
	void OpenExplorerProperties(std::string_view filePath);

	std::string ResolveFileLink(std::string_view filePath);

	void FuckUpWindowsPath(std::string& path);
	void SanitizePath(std::string& path);

	std::string GetWorkingDirectory();
	void SetWorkingDirectory(std::string_view path);

	std::string Combine(std::string_view pathA, std::string_view pathB);

	std::string_view GetFileName(std::string_view filePath, bool extension = true);
	std::string_view GetDirectory(std::string_view filePath);
	std::string_view GetFileExtension(std::string_view filePath);

	std::vector<std::string> GetFiles(std::string_view directory);

	bool ReadAllBytes(std::string_view filePath, void* buffer, size_t bufferSize);

	template <typename T>
	inline bool ReadAllBytes(std::string_view filePath, T* buffer) { return ReadAllBytes(filePath, buffer, sizeof(T)); };

	bool WriteAllBytes(std::string_view filePath, const void* buffer, size_t bufferSize);

	template <typename T>
	inline bool WriteAllBytes(std::string_view filePath, const T& buffer) { return WriteAllBytes(filePath, &buffer, sizeof(T)); };

	bool WriteAllBytes(std::string_view filePath, const std::vector<uint8_t>& buffer);
	bool ReadAllLines(std::string_view filePath, std::vector<std::string>* buffer);
}

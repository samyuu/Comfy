#pragma once
#include "Core/CoreTypes.h"
#include "FileSystem/FileReader.h"

namespace FileSystem
{
	const Vector<String> AllFilesFilter = { "All Files (*.*)", "*.*" };

	bool CreateDirectory(const WideString& filePath);

	bool IsFilePath(const String& filePath);
	bool IsFilePath(const WideString& filePath);

	bool IsDirectoryPath(const String& directory);
	bool IsDirectoryPath(const WideString& directory);

	bool IsPathRelative(const String& path);
	bool IsPathRelative(const WideString& path);

	bool FileExists(const String& filePath);
	bool FileExists(const WideString& filePath);

	bool DirectoryExists(const String& directory);
	bool DirectoryExists(const WideString& directory);

	bool CreateOpenFileDialog(WideString& outFilePath, const char* title = nullptr, const char* directory = nullptr, const Vector<String>& filter = AllFilesFilter);
	bool CreateSaveFileDialog(WideString& outFilePath, const char* title = nullptr, const char* directory = nullptr, const Vector<String>& filter = AllFilesFilter);

	void OpenWithDefaultProgram(const WideString& filePath);
	void OpenInExplorer(const WideString& filePath);
	void OpenExplorerProperties(const WideString& filePath);

	void FuckUpWindowsPath(String& path);
	void FuckUpWindowsPath(WideString& path);

	void SanitizePath(String& path);
	void SanitizePath(WideString& path);

	String GetWorkingDirectory();
	WideString GetWorkingDirectoryW();

	void SetWorkingDirectory(const String& value);
	void SetWorkingDirectoryW(const WideString& value);

	String Combine(const String& pathA, const String& pathB);
	WideString Combine(const WideString& pathA, const WideString& pathB);

	String GetFileName(const String& filePath, bool extension = true);
	WideString GetFileName(const WideString& filePath, bool extension = true);

	String GetDirectory(const String& filePath);
	WideString GetDirectory(const WideString& filePath);

	String GetFileExtension(const String& filePath);
	WideString GetFileExtension(const WideString& filePath);

	Vector<String> GetFiles(const String& directory);
	Vector<WideString> GetFiles(const WideString& directory);

	bool WriteAllBytes(const String& filePath, const Vector<uint8_t>& buffer);
	bool WriteAllBytes(const WideString& filePath, const Vector<uint8_t>& buffer);

	bool ReadAllLines(const String& filePath, Vector<String>* buffer);
	bool ReadAllLines(const WideString& filePath, Vector<WideString>* buffer);
}

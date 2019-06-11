#pragma once
#include <string>

bool FileExists(const std::string& filePath);
bool FileExists(const std::wstring& filePath);

std::string GetFileName(const std::string& filePath, bool extension = true);
std::wstring GetFileName(const std::wstring& filePath, bool extension = true);

std::string GetFileExtension(const std::string& filePath);
std::wstring GetFileExtension(const std::wstring& filePath);
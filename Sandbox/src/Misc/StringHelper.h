#pragma once
#include <string>

bool StartsWith(const std::string& string, const std::string& prefix);
bool StartsWith(const std::wstring& string, const std::wstring& prefix);

bool StartsWithInsensitive(const std::string& string, const std::string& prefix);
bool StartsWithInsensitive(const std::wstring& string, const std::wstring& prefix);

bool EndsWith(const std::string& string, char suffix);
bool EndsWith(const std::wstring& string, wchar_t suffix);

bool EndsWith(const std::string& string, const std::string& suffix);
bool EndsWith(const std::wstring& string, const std::wstring& suffix);

bool EndsWithInsensitive(const std::string& string, const std::string& suffix);
bool EndsWithInsensitive(const std::wstring& string, const std::wstring& suffix);
#pragma once
#include <string>

void TrimLeft(std::string &string);
void TrimRight(std::string &string);
void Trim(std::string &string);

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

std::wstring Utf8ToUtf16(const std::string& string);
std::string Utf16ToUtf8(const std::wstring& string);

#pragma once
#include "Core/CoreTypes.h"

void TrimLeft(std::string& string);
void TrimRight(std::string& string);
void Trim(std::string& string);

bool StartsWith(const std::string_view string, const std::string_view prefix);
bool StartsWith(const std::wstring_view string, const std::wstring_view prefix);

bool StartsWithInsensitive(const std::string_view string, const std::string_view prefix);
bool StartsWithInsensitive(const std::wstring_view string, const std::wstring_view prefix);

bool EndsWith(const std::string_view string, char suffix);
bool EndsWith(const std::wstring_view string, wchar_t suffix);

bool EndsWith(const std::string_view string, const std::string_view suffix);
bool EndsWith(const std::wstring_view string, const std::wstring_view suffix);

bool EndsWithInsensitive(const std::string_view string, const std::string_view suffix);
bool EndsWithInsensitive(const std::wstring_view string, const std::wstring_view suffix);

std::wstring Utf8ToUtf16(const std::string& string);
std::string Utf16ToUtf8(const std::wstring& string);

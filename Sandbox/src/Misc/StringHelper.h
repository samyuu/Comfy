#pragma once
#include <string>

bool EndsWith(const std::string& string, const std::string& ending);
bool EndsWith(const std::wstring& string, const std::wstring& ending);

bool EndsWithCaseInsensitive(const std::string& string, const std::string& ending);
bool EndsWithCaseInsensitive(const std::wstring& string, const std::wstring& ending);
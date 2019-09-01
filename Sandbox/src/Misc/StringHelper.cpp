#include "StringHelper.h"
#include <algorithm>
#include <windows.h>

static bool IsWhiteSpace(char character)
{
	return character == ' ' || character == '\t' || character == '\n' || character == '\r';
}

static bool CaseInsenitiveComparison(char a, char b)
{
	return tolower(a) == tolower(b);
}

static bool CaseInsenitiveWideComparison(wchar_t a, wchar_t b)
{
	return tolower(a) == tolower(b);
};

void TrimLeft(String &string)
{
	string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](int ch) { return !IsWhiteSpace(ch); }));
}

void TrimRight(String &string)
{
	string.erase(std::find_if(string.rbegin(), string.rend(), [](int ch) { return !IsWhiteSpace(ch); }).base(), string.end());
}

void Trim(String &string)
{
	TrimLeft(string);
	TrimRight(string);
}

bool StartsWith(const String& string, const String& prefix)
{
	return string.find(prefix) == 0;
}

bool StartsWith(const WideString& string, const WideString& prefix)
{
	return string.find(prefix) == 0;
}

bool StartsWithInsensitive(const String& string, const String& prefix)
{
	return std::equal(prefix.begin(), prefix.end(), string.begin());
}

bool StartsWithInsensitive(const WideString& string, const WideString& prefix)
{
	return std::equal(prefix.begin(), prefix.end(), string.begin());
}

bool EndsWith(const String& string, char suffix)
{
	return string.size() > 0 && string.back() == suffix;
}

bool EndsWith(const WideString& string, wchar_t suffix)
{
	return string.size() > 0 && string.back() == suffix;
}

bool EndsWith(const String& string, const String& suffix)
{
	if (suffix.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend());
}

bool EndsWith(const WideString& string, const WideString& suffix)
{
	if (suffix.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend());
}

bool EndsWithInsensitive(const String& string, const String& suffix)
{
	if (suffix.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend(), CaseInsenitiveComparison);
}

bool EndsWithInsensitive(const WideString& string, const WideString& suffix)
{
	if (suffix.size() > string.size())
		return false;

	return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend(), CaseInsenitiveWideComparison);
}

WideString Utf8ToUtf16(const String& string)
{
	WideString utf16String;
	
	int utf16Length = ::MultiByteToWideChar(CP_UTF8, NULL, string.c_str(), -1, nullptr, 0) - 1;
	if (utf16Length > 0)
	{
		utf16String.resize(utf16Length);
		::MultiByteToWideChar(CP_UTF8, NULL, string.c_str(), static_cast<int>(string.length()), utf16String.data(), utf16Length);
	}
	
	return utf16String;
}

String Utf16ToUtf8(const WideString& string)
{
	String utf8String;

	int utf8Length = ::WideCharToMultiByte(CP_UTF8, NULL, string.c_str(), -1, nullptr, 0, nullptr, nullptr) - 1;
	if (utf8Length > 0)
	{
		utf8String.resize(utf8Length);
		::WideCharToMultiByte(CP_UTF8, NULL, string.c_str(), static_cast<int>(string.length()), utf8String.data(), utf8Length, nullptr, nullptr);
	}

	return utf8String;
}

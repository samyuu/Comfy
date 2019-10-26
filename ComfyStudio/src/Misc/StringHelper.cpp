#include "StringHelper.h"
#include "Core/Win32/ComfyWindows.h"
#include <algorithm>

namespace Utilities
{
	namespace
	{
		constexpr bool IsWhiteSpace(const char character)
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r';
		}

		bool CaseInsenitiveComparison(const char a, const char b)
		{
			return tolower(a) == tolower(b);
		}

		bool CaseInsenitiveWideComparison(const wchar_t a, const wchar_t b)
		{
			return tolower(a) == tolower(b);
		};
	}

	void TrimLeft(std::string& string)
	{
		string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](int ch) { return !IsWhiteSpace(ch); }));
	}

	void TrimRight(std::string& string)
	{
		string.erase(std::find_if(string.rbegin(), string.rend(), [](int ch) { return !IsWhiteSpace(ch); }).base(), string.end());
	}

	void Trim(std::string& string)
	{
		TrimLeft(string);
		TrimRight(string);
	}

	bool MatchesInsensitive(const std::string_view stringA, const std::string_view stringB)
	{
		return std::equal(stringA.begin(), stringA.end(), stringB.begin(), stringB.end(), CaseInsenitiveComparison);
	}

	bool MatchesInsensitive(const std::wstring_view stringA, const std::wstring_view stringB)
	{
		return std::equal(stringA.begin(), stringA.end(), stringB.begin(), stringB.end(), CaseInsenitiveComparison);
	}

	bool StartsWith(const std::string_view string, char suffix)
	{
		return !string.empty() && string.front() == suffix;
	}

	bool StartsWith(const std::wstring_view string, wchar_t suffix)
	{
		return !string.empty() && string.front() == suffix;
	}

	bool StartsWith(const std::string_view string, const std::string_view prefix)
	{
		return string.find(prefix) == 0;
	}

	bool StartsWith(const std::wstring_view string, const std::wstring_view prefix)
	{
		return string.find(prefix) == 0;
	}

	bool StartsWithInsensitive(const std::string_view string, const std::string_view prefix)
	{
		return std::equal(prefix.begin(), prefix.end(), string.begin());
	}

	bool StartsWithInsensitive(const std::wstring_view string, const std::wstring_view prefix)
	{
		return std::equal(prefix.begin(), prefix.end(), string.begin());
	}

	bool EndsWith(const std::string_view string, char suffix)
	{
		return !string.empty() && string.back() == suffix;
	}

	bool EndsWith(const std::wstring_view string, wchar_t suffix)
	{
		return !string.empty() && string.back() == suffix;
	}

	bool EndsWith(const std::string_view string, const std::string_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend());
	}

	bool EndsWith(const std::wstring_view string, const std::wstring_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend());
	}

	bool EndsWithInsensitive(const std::string_view string, const std::string_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend(), CaseInsenitiveComparison);
	}

	bool EndsWithInsensitive(const std::wstring_view string, const std::wstring_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend(), CaseInsenitiveWideComparison);
	}

	std::wstring Utf8ToUtf16(const std::string& string)
	{
		std::wstring utf16String;

		const int utf16Length = ::MultiByteToWideChar(CP_UTF8, NULL, string.c_str(), -1, nullptr, 0) - 1;
		if (utf16Length > 0)
		{
			utf16String.resize(utf16Length);
			::MultiByteToWideChar(CP_UTF8, NULL, string.c_str(), static_cast<int>(string.length()), utf16String.data(), utf16Length);
		}

		return utf16String;
	}

	std::string Utf16ToUtf8(const std::wstring& string)
	{
		std::string utf8String;

		const int utf8Length = ::WideCharToMultiByte(CP_UTF8, NULL, string.c_str(), -1, nullptr, 0, nullptr, nullptr) - 1;
		if (utf8Length > 0)
		{
			utf8String.resize(utf8Length);
			::WideCharToMultiByte(CP_UTF8, NULL, string.c_str(), static_cast<int>(string.length()), utf8String.data(), utf8Length, nullptr, nullptr);
		}

		return utf8String;
	}
}
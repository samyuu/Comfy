#include "StringHelper.h"
#include "Core/Win32/ComfyWindows.h"
#include <algorithm>

namespace Comfy::Utilities
{
	namespace
	{
		constexpr bool IsWhiteSpace(const char character)
		{
			return character == ' ' || character == '\t' || character == '\n' || character == '\r';
		}

		bool CaseInsenitiveComparison(const int a, const int b)
		{
			return tolower(a) == tolower(b);
		}
	}

	void TrimLeft(std::string& string)
	{
		string.erase(string.begin(), std::find_if(string.begin(), string.end(), [](const char character) { return !IsWhiteSpace(character); }));
	}

	void TrimRight(std::string& string)
	{
		string.erase(std::find_if(string.rbegin(), string.rend(), [](const char character) { return !IsWhiteSpace(character); }).base(), string.end());
	}

	void Trim(std::string& string)
	{
		TrimLeft(string);
		TrimRight(string);
	}

	bool MatchesInsensitive(std::string_view stringA, std::string_view stringB)
	{
		return std::equal(stringA.begin(), stringA.end(), stringB.begin(), stringB.end(), CaseInsenitiveComparison);
	}

	bool Contains(std::string_view stringA, std::string_view stringB)
	{
		return stringA.find(stringB) != std::string::npos;
	}

	bool StartsWith(std::string_view string, char suffix)
	{
		return !string.empty() && string.front() == suffix;
	}

	bool StartsWith(std::string_view string, std::string_view prefix)
	{
		return string.find(prefix) == 0;
	}

	bool StartsWithInsensitive(std::string_view string, std::string_view prefix)
	{
		// return std::equal(prefix.begin(), prefix.end(), string.begin());
		return (string.size() >= prefix.size() && MatchesInsensitive(string.substr(0, prefix.size()), prefix));
	}

	bool EndsWith(std::string_view string, char suffix)
	{
		return !string.empty() && string.back() == suffix;
	}

	bool EndsWith(std::string_view string, std::string_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend());
	}

	bool EndsWithInsensitive(std::string_view string, std::string_view suffix)
	{
		if (suffix.size() > string.size())
			return false;

		return std::equal(string.rbegin(), string.rbegin() + suffix.size(), suffix.rbegin(), suffix.rend(), CaseInsenitiveComparison);
	}
}

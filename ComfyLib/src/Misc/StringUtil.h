#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ASCII.h"
#include "UTF8.h"
#include <algorithm>

namespace Comfy::Util
{
	COMFY_NODISCARD constexpr bool CaseInsenitiveComparison(char characterA, char characterB)
	{
		// NOTE: Because tolower() isn't constexpr. None of this is properly unicode compliant but that shouldn't be an issue for now
		return (ASCII::ToLowerCase(characterA) == ASCII::ToLowerCase(characterB));
	}

	COMFY_NODISCARD constexpr std::string_view TrimLeft(std::string_view string)
	{
		const auto firstNonWhiteSpace = string.find_first_not_of(ASCII::WhiteSpaceCharactersString);
		return (firstNonWhiteSpace == std::string_view::npos) ? string : string.substr(firstNonWhiteSpace);
	}

	COMFY_NODISCARD constexpr std::string_view TrimRight(std::string_view string)
	{
		const auto lastNonWhiteSpace = string.find_last_not_of(ASCII::WhiteSpaceCharactersString);
		return (lastNonWhiteSpace == std::string_view::npos) ? string : string.substr(0, lastNonWhiteSpace + 1);
	}

	COMFY_NODISCARD constexpr std::string_view Trim(std::string_view string)
	{
		return TrimRight(TrimLeft(string));
	}

	COMFY_NODISCARD constexpr bool Matches(std::string_view stringA, std::string_view stringB)
	{
		if (stringA.size() != stringB.size())
			return false;

		for (auto i = 0; i < stringA.size(); i++)
		{
			if (stringA[i] != stringB[i])
				return false;
		}

		return true;
	}

	COMFY_NODISCARD constexpr bool MatchesInsensitive(std::string_view stringA, std::string_view stringB)
	{
		if (stringA.size() != stringB.size())
			return false;

		for (auto i = 0; i < stringA.size(); i++)
		{
			if (!CaseInsenitiveComparison(stringA[i], stringB[i]))
				return false;
		}

		return true;
	}

	COMFY_NODISCARD constexpr bool Contains(std::string_view stringA, std::string_view stringB)
	{
		return (stringA.find(stringB) != std::string::npos);
	}

	COMFY_NODISCARD constexpr bool StartsWith(std::string_view string, char prefix)
	{
		return (!string.empty() && string.front() == prefix);
	}

	COMFY_NODISCARD constexpr bool StartsWith(std::string_view string, std::string_view prefix)
	{
		return (string.size() >= prefix.size() && Matches(string.substr(0, prefix.size()), prefix));
	}

	COMFY_NODISCARD constexpr bool StartsWithInsensitive(std::string_view string, std::string_view prefix)
	{
		return (string.size() >= prefix.size() && MatchesInsensitive(string.substr(0, prefix.size()), prefix));
	}

	COMFY_NODISCARD constexpr bool EndsWith(std::string_view string, char suffix)
	{
		return (!string.empty() && string.back() == suffix);
	}

	COMFY_NODISCARD constexpr bool EndsWith(std::string_view string, std::string_view suffix)
	{
		return (string.size() >= suffix.size() && Matches(string.substr(string.size() - suffix.size()), suffix));
	}

	COMFY_NODISCARD constexpr bool EndsWithInsensitive(std::string_view string, std::string_view suffix)
	{
		return (string.size() >= suffix.size() && MatchesInsensitive(string.substr(string.size() - suffix.size()), suffix));
	}

	COMFY_NODISCARD constexpr std::string_view StripPrefix(std::string_view string, std::string_view prefix)
	{
		return Util::StartsWith(string, prefix) ? string.substr(prefix.size(), string.size() - prefix.size()) : string;
	}

	COMFY_NODISCARD constexpr std::string_view StripPrefixInsensitive(std::string_view string, std::string_view prefix)
	{
		return Util::StartsWithInsensitive(string, prefix) ? string.substr(prefix.size(), string.size() - prefix.size()) : string;
	}

	COMFY_NODISCARD constexpr std::string_view StripSuffix(std::string_view string, std::string_view suffix)
	{
		return EndsWith(string, suffix) ? string.substr(0, string.size() - suffix.size()) : string;
	}

	COMFY_NODISCARD constexpr std::string_view StripSuffixInsensitive(std::string_view string, std::string_view suffix)
	{
		return EndsWithInsensitive(string, suffix) ? string.substr(0, string.size() - suffix.size()) : string;
	}

	COMFY_NODISCARD inline std::string ToLowerCopy(std::string string)
	{
		for (auto& character : string)
			character = ASCII::ToLowerCase(character);
		return string;
	}

	COMFY_NODISCARD inline std::string ToUpperCopy(std::string string)
	{
		for (auto& character : string)
			character = ASCII::ToUpperCase(character);
		return string;
	}

	COMFY_NODISCARD inline std::string ToSnakeCaseLowerCopy(std::string string)
	{
		auto result = ToLowerCopy(string);
		std::replace(result.begin(), result.end(), ' ', '_');
		return result;
	}

	COMFY_NODISCARD inline std::string ToSnakeCaseUpperCopy(std::string string)
	{
		auto result = ToUpperCopy(string);
		std::replace(result.begin(), result.end(), ' ', '_');
		return result;
	}
}

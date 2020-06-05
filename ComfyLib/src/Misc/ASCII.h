#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::ASCII
{
	constexpr std::string_view WhiteSpaceCharactersString = " \t\r\n";
	constexpr std::array<char, 4> WhiteSpaceCharactersArray = { ' ', '\t', '\r', '\n' };

	constexpr char CaseDifference = ('A' - 'a');

	constexpr char LowerCaseMin = 'a', LowerCaseMax = 'z';
	constexpr char UpperCaseMin = 'A', UpperCaseMax = 'Z';

	COMFY_NODISCARD constexpr bool IsWhiteSpace(char character)
	{
		return
			character == WhiteSpaceCharactersArray[0] ||
			character == WhiteSpaceCharactersArray[1] ||
			character == WhiteSpaceCharactersArray[2] ||
			character == WhiteSpaceCharactersArray[3];
	}

	COMFY_NODISCARD constexpr bool IsLowerCase(char character)
	{
		return (character >= LowerCaseMin && character <= LowerCaseMax);
	}

	COMFY_NODISCARD constexpr bool IsUpperCase(char character)
	{
		return (character >= UpperCaseMin && character <= UpperCaseMax);
	}

	COMFY_NODISCARD constexpr char ToLowerCase(char character)
	{
		return IsUpperCase(character) ? (character - CaseDifference) : character;
	}

	COMFY_NODISCARD constexpr char ToUpperCase(char character)
	{
		return IsLowerCase(character) ? (character + CaseDifference) : character;
	}
}

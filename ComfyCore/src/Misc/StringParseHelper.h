#pragma once
#include "Types.h"
#include "Core/CoreTypes.h"
#include <charconv>

namespace Utilities::StringParsing
{
	std::string_view GetLine(const char* textBuffer);
	std::string_view GetWord(const char* textBuffer);
	
	void AdvanceToNextLine(const char*& textBuffer);
	std::string_view GetLineAdvanceToNextLine(const char*& textBuffer);

	template <typename T>
	T ParseType(std::string_view string)
	{
		T value = {};
		auto result = std::from_chars(string.data(), string.data() + string.size(), value);
		return value;
	}

	template <typename T, size_t Size>
	std::array<T, Size> ParseTypeArray(std::string_view string)
	{
		std::array<T, Size> value = {};

		for (size_t i = 0; i < Size; i++)
		{
			auto word = GetWord(string.data());
			value[i] = ParseType<T>(word);

			if (i + 1 < Size)
				string = string.substr(word.size() + 1);
		}

		return value;
	}
}

#pragma once
#include "Misc/StringParseHelper.h"

namespace Utilities::StringParsing
{
	constexpr size_t MaximumNestedProperties = 16;

	class TextDatabaseParser
	{
	private:
		struct ParserState
		{
			std::array<std::string_view, MaximumNestedProperties> PropertiesBuffer;

			int CurrentPropertyIndex;
			std::string_view CurrentProperty;
			std::string_view CurrentValueString;
		} state;

	protected:
		inline void StateParseNewLinePropertiesAndValue(std::string_view line)
		{
			const char* currentLinePosition = line.data();

			state.PropertiesBuffer = {};
			for (auto& property : state.PropertiesBuffer)
			{
				property = StringParsing::GetPropertyAdvanceToNextProperty(currentLinePosition);
				if (property.empty())
					break;
			}

			state.CurrentValueString = StringParsing::GetValue(currentLinePosition);

			state.CurrentPropertyIndex = 0;
			AdvanceProperty();
		}

	protected:
		inline std::string_view ParseValueString()
		{
			return state.CurrentValueString;
		}

		template <typename T>
		inline T ParseValueString()
		{
			return StringParsing::ParseType<T>(ParseValueString());
		}

		template <typename T>
		inline T ParseEnumValueString()
		{
			return static_cast<T>(StringParsing::ParseType<uint32_t>(ParseValueString()));
		}

		inline void AdvanceProperty()
		{
			state.CurrentProperty = state.PropertiesBuffer[state.CurrentPropertyIndex++];
		}

		inline bool IsLastProperty()
		{
			return state.PropertiesBuffer[state.CurrentPropertyIndex - 1].empty();
		}

		inline bool CompareProperty(std::string_view property)
		{
			if (property == state.CurrentProperty)
			{
				AdvanceProperty();
				return true;
			}

			return false;
		}

		template <typename T>
		inline bool TryParseLength(std::vector<T>& vector, std::string_view lengthIdentifier = "length")
		{
			if (CompareProperty(lengthIdentifier))
			{
				vector.resize(ParseValueString<uint32_t>());
				return true;
			}

			return false;
		}

		inline uint32_t ParseAdvanceIndexProperty()
		{
			auto index = StringParsing::ParseType<uint32_t>(state.CurrentProperty);
			AdvanceProperty();
			return index;
		}

		template <typename T, size_t TSize, bool TParentheses = true>
		inline std::array<T, TSize> ParseCommaSeparatedArray()
		{
			// NOTE: Optionally remove surrounding parentheses
			std::string_view commaSeparatedData = TParentheses ? (state.CurrentValueString.substr(1, state.CurrentValueString.size() - 2)) : state.CurrentValueString;

			int dataIndex = 0;
			std::array<T, TSize> rawData = {};

			int lastCommaIndex = 0;
			for (int i = 0; i < commaSeparatedData.size(); i++)
			{
				if (commaSeparatedData[i] == ',')
				{
					std::string_view dataString = commaSeparatedData.substr(lastCommaIndex, i - lastCommaIndex);
					lastCommaIndex = i + 1;

					rawData[dataIndex++] = StringParsing::ParseType<float>(dataString);
				}
			}

			rawData[dataIndex] = StringParsing::ParseType<float>(commaSeparatedData.substr(lastCommaIndex));
			return rawData;
		}
	};
}

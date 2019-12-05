#include "StringParseHelper.h"

namespace Utilities::StringParsing
{
	std::string_view GetLine(const char* textBuffer)
	{
		const char* startOfLine = textBuffer;
		const char* endOfLine = textBuffer;

		while (*endOfLine != '\0' && *endOfLine != '\r' && *endOfLine != '\n')
			endOfLine++;

		const size_t lineLength = endOfLine - startOfLine;
		return std::string_view(textBuffer, lineLength);
	}

	std::string_view GetWord(const char* textBuffer)
	{
		const char* startOfWord = textBuffer;
		const char* endOfWord = textBuffer;

		while (*endOfWord != '\0' && *endOfWord != ' ' && *endOfWord != '\r' && *endOfWord != '\n')
			endOfWord++;

		const size_t lineLength = endOfWord - startOfWord;
		return std::string_view(textBuffer, lineLength);
	}

	void AdvanceToNextLine(const char*& textBuffer)
	{
		while (*textBuffer != '\0')
		{
			if (*textBuffer == '\r')
			{
				textBuffer++;

				if (*textBuffer == '\n')
					textBuffer++;

				return;
			}

			if (*textBuffer == '\n')
			{
				textBuffer++;
				return;
			}

			textBuffer++;
		}
	}
}

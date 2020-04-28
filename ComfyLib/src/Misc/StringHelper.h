#pragma once
#include "CoreTypes.h"
#include "UTF8.h"

namespace Comfy
{
	namespace Utilities
	{
		void TrimLeft(std::string& string);
		void TrimRight(std::string& string);
		void Trim(std::string& string);

		bool MatchesInsensitive(std::string_view stringA, std::string_view stringB);

		bool Contains(std::string_view stringA, std::string_view stringB);

		bool StartsWith(std::string_view string, char suffix);
		bool StartsWith(std::string_view string, std::string_view prefix);

		bool StartsWithInsensitive(std::string_view string, std::string_view prefix);

		bool EndsWith(std::string_view string, char suffix);
		bool EndsWith(std::string_view string, std::string_view suffix);

		bool EndsWithInsensitive(std::string_view string, std::string_view suffix);
	}

	using namespace Utilities;
}

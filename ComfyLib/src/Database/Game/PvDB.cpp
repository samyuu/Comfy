#include "PvDB.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "Misc/TextDatabaseParser.h"
#include "IO/Stream/Manipulator/StreamWriter.h"
#include <memory_resource>

namespace Comfy::Database
{
	using namespace Util;

	namespace
	{
		struct PvDBParser final : public StringParsing::TextDatabaseParser
		{
		private:
			i32 lastParsedID = -1, lastIDEntryIndex = 0;

		private:
			// TODO:
			bool TryParseCommon(PvDBCommonEntry&)
			{
			}

			void ParseProperties(PvDB& pvDB)
			{
				const auto pvIDProperty = PeekCurrentProperty();
				AdvanceProperty();

				// BUG: Starting { "\npv_999", "bpm" }
				static constexpr std::string_view idDummyString = "pv_xxx", idPrefix = "pv_";
				if (pvIDProperty.length() < idDummyString.size() || !StartsWith(pvIDProperty, idPrefix))
					return;

				const auto pvID = StringParsing::ParseType<i32>(pvIDProperty.substr(idPrefix.size()));
				if (lastParsedID < 0 || lastParsedID != pvID)
				{
					lastParsedID = pvID;
					lastIDEntryIndex = static_cast<i32>(pvDB.Entries.size());
					pvDB.Entries.push_back(std::make_unique<PvDBEntry>());
				}

				auto& pvEntry = *pvDB.Entries[lastIDEntryIndex];
				pvEntry.ID = pvID;

				if (CompareProperty("song_name"))
				{
					pvEntry.SongName = ParseValueString();
				}
				else
				{

				}
			}

		public:
			bool Parse(PvDB& pvDB, const char* startOfTextBuffer, const char* endOfTextBuffer)
			{
				const char* textBuffer = endOfTextBuffer;

				while (textBuffer >= startOfTextBuffer)
				{
					const std::string_view currentLine = StringParsing::AdvanceToStartOfPreviousLineGetNonCommentLine(textBuffer, startOfTextBuffer);
					if (textBuffer <= startOfTextBuffer)
						break;

					StateParseNewLinePropertiesAndValue(currentLine);
					ParseProperties(pvDB);
				}

				std::sort(pvDB.Entries.begin(), pvDB.Entries.end(), [&](auto& a, auto& b) { return (a->ID < b->ID); });
				return true;
			}
		};
	}

	void PvDB::Parse(const u8* buffer, size_t bufferSize)
	{
		// TODO:
		const char* const startOfTextBuffer = reinterpret_cast<const char*>(buffer);
		const char* const endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		PvDBParser parser;
		parser.Parse(*this, startOfTextBuffer, endOfTextBuffer);
	}

	IO::StreamResult PvDB::Write(IO::StreamWriter& writer)
	{
		// TODO:
		for (auto& entry : Entries)
		{
			char b[255];
			writer.WriteBuffer(b, sprintf_s(b, "pv_%03d.song_name=%s\n", entry->ID, entry->SongName.c_str()));
		}

		return IO::StreamResult::Success;
	}
}

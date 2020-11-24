#include "SfxDB.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "Misc/TextDatabaseParser.h"

namespace Comfy::Database
{
	using namespace Util;

	namespace
	{
		struct SfxDBParser final : public StringParsing::TextDatabaseParser
		{
		private:
			void ParseProperties(SfxDB& sfxDB)
			{
				if (CompareProperty("volume_bias"))
					sfxDB.VolumeBias = ParseValueString<f32>();
				else if (CompareProperty("version_name"))
					sfxDB.VersionName = ParseValueString();
				else if (CompareProperty("version_date"))
					sfxDB.VersionDate = ParseValueString();
				else if (!TryParseLength(sfxDB.Entries, "max"))
				{
					auto& entry = sfxDB.Entries[ParseAdvanceIndexProperty() - 1];

					if (CompareProperty("file_name"))
						entry.FileName = ParseValueString();
					else if (CompareProperty("name"))
						entry.Name = ParseValueString();
					else if (CompareProperty("volume"))
						entry.Volume = ParseValueString<f32>();
					else if (CompareProperty("loop_start"))
						entry.LoopStartFrame = ParseValueString<i32>();
					else if (CompareProperty("loop_end"))
						entry.LoopEndFrame = ParseValueString<i32>();
					else if (CompareProperty("release_time"))
						entry.ReleaseFrame = ParseValueString<i32>();
				}
			}

		public:
			bool Parse(SfxDB& sfxDB, const char* startOfTextBuffer, const char* endOfTextBuffer)
			{
				const char* textBuffer = endOfTextBuffer;

				while (textBuffer >= startOfTextBuffer)
				{
					const std::string_view currentLine = StringParsing::AdvanceToStartOfPreviousLineGetNonCommentLine(textBuffer, startOfTextBuffer);
					if (textBuffer <= startOfTextBuffer)
						break;

					StateParseNewLinePropertiesAndValue(currentLine);
					ParseProperties(sfxDB);
				}

				return true;
			}
		};
	}

	void SfxDB::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* const startOfTextBuffer = reinterpret_cast<const char*>(buffer);
		const char* const endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		SfxDBParser parser;
		parser.Parse(*this, startOfTextBuffer, endOfTextBuffer);
	}
}

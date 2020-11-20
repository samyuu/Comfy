#include "GmBtnSfxDB.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"
#include "Misc/TextDatabaseParser.h"

namespace Comfy::Database
{
	using namespace Util;

	namespace
	{
		struct GmBtnSfxDBParser final : public StringParsing::TextDatabaseParser
		{
		private:
			void ParseProperties(GmBtnSfxDB& btnSfxDB)
			{
				bool isChainSlide = false;

				if (CompareProperty("kind"))
					btnSfxDB.Type = ParseEnumValueString<GmBtnSfxType>();
				else if (CompareProperty("btn_se") || CompareProperty("slide_se") || (isChainSlide = CompareProperty("chainslide_se")) || CompareProperty("slidertouch_se"))
				{
					if (CompareProperty("data_list"))
						TryParseLength(btnSfxDB.Entries, "length");
					else
					{
						auto& entry = btnSfxDB.Entries[ParseAdvanceIndexProperty()];

						if (CompareProperty("st_year"))
							entry.StartDate.Year = ParseValueString<i32>();
						else if (CompareProperty("st_month"))
							entry.StartDate.Month = ParseValueString<i32>();
						else if (CompareProperty("st_day"))
							entry.StartDate.Day = ParseValueString<i32>();
						else if (CompareProperty("ed_year"))
							entry.EndDate.Year = ParseValueString<i32>();
						else if (CompareProperty("ed_month"))
							entry.EndDate.Month = ParseValueString<i32>();
						else if (CompareProperty("ed_day"))
							entry.EndDate.Day = ParseValueString<i32>();
						else if (CompareProperty("id"))
							entry.ID = ParseValueString<u32>();
						else if (CompareProperty("sort_index"))
							entry.SortIndex = ParseValueString<u32>();
						else if (CompareProperty("name"))
							entry.DisplayName = ParseValueString();
						else if (CompareProperty("se_name"))
							entry.SfxName = ParseValueString();
						else if (isChainSlide)
						{
							if (CompareProperty("first_se_name"))
								entry.Chain.SfxNameFirst = ParseValueString();
							else if (CompareProperty("sub_se_name"))
								entry.Chain.SfxNameSub = ParseValueString();
							else if (CompareProperty("success_se_name"))
								entry.Chain.SfxNameSuccess = ParseValueString();
							else if (CompareProperty("failure_se_name"))
								entry.Chain.SfxNameFailure = ParseValueString();
						}
					}
				}
			}

		public:
			bool Parse(GmBtnSfxDB& btnSfxDB, const char* startOfTextBuffer, const char* endOfTextBuffer)
			{
				const char* textBuffer = endOfTextBuffer;

				while (textBuffer >= startOfTextBuffer)
				{
					const std::string_view currentLine = StringParsing::AdvanceToStartOfPreviousLineGetNonCommentLine(textBuffer, startOfTextBuffer);
					if (textBuffer <= startOfTextBuffer)
						break;

					StateParseNewLinePropertiesAndValue(currentLine);
					ParseProperties(btnSfxDB);
				}

				return true;
			}
		};
	}

	void GmBtnSfxDB::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* const startOfTextBuffer = reinterpret_cast<const char*>(buffer);
		const char* const endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		GmBtnSfxDBParser parser;
		parser.Parse(*this, startOfTextBuffer, endOfTextBuffer);
	}
}

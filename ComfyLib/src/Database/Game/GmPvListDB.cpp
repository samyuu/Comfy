#include "GmPvListDB.h"
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
		struct GmPvListDBParser final : public StringParsing::TextDatabaseParser
		{
		private:
			bool TryParseDifficultyArray(GmPvDifficultyEntries& output)
			{
				if (CompareProperty("length"))
				{
					output.Count = ParseValueString<i32>();
				}
				else
				{
					auto& entry = output.Editions[ParseAdvanceIndexProperty()];
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
					else if (CompareProperty("edition"))
						entry.Edition = ParseValueString<i32>();
					else if (CompareProperty("ver"))
						entry.Version = ParseValueString<i32>();
				}

				return true;
			}

			void ParseProperties(GmPvListDB& pvListDB)
			{
				if (CompareProperty("pv_list"))
				{
					if (CompareProperty("data_list"))
					{
						TryParseLength(pvListDB.Entries, "length");
					}
					else
					{
						auto& entry = pvListDB.Entries[ParseAdvanceIndexProperty()];
						if (CompareProperty("adv_demo_st_year"))
							entry.AdvDemoStartDate.Year = ParseValueString<i32>();
						else if (CompareProperty("adv_demo_st_month"))
							entry.AdvDemoStartDate.Month = ParseValueString<i32>();
						else if (CompareProperty("adv_demo_st_day"))
							entry.AdvDemoStartDate.Day = ParseValueString<i32>();
						else if (CompareProperty("adv_demo_ed_year"))
							entry.AdvDemoEndDate.Year = ParseValueString<i32>();
						else if (CompareProperty("adv_demo_ed_month"))
							entry.AdvDemoEndDate.Month = ParseValueString<i32>();
						else if (CompareProperty("adv_demo_ed_day"))
							entry.AdvDemoEndDate.Day = ParseValueString<i32>();
						else if (CompareProperty("id"))
							entry.ID = ParseValueString<u32>();
						else if (CompareProperty("ignore"))
							entry.Ignore = ParseValueString<u32>();
						else if (CompareProperty("name"))
							entry.Name = ParseValueString();
						else if (CompareProperty("easy"))
							TryParseDifficultyArray(entry.Easy);
						else if (CompareProperty("normal"))
							TryParseDifficultyArray(entry.Normal);
						else if (CompareProperty("hard"))
							TryParseDifficultyArray(entry.Hard);
						else if (CompareProperty("extreme"))
							TryParseDifficultyArray(entry.Extreme);
						else if (CompareProperty("encore"))
							TryParseDifficultyArray(entry.Encore);
					}
				}
			}

		public:
			bool Parse(GmPvListDB& pvListDB, const char* startOfTextBuffer, const char* endOfTextBuffer)
			{
				const char* textBuffer = endOfTextBuffer;

				while (textBuffer >= startOfTextBuffer)
				{
					const std::string_view currentLine = StringParsing::AdvanceToStartOfPreviousLineGetNonCommentLine(textBuffer, startOfTextBuffer);
					if (textBuffer <= startOfTextBuffer)
						break;

					StateParseNewLinePropertiesAndValue(currentLine);
					ParseProperties(pvListDB);
				}

				return true;
			}
		};
	}

	void GmPvListDB::Parse(const u8* buffer, size_t bufferSize)
	{
		const char* const startOfTextBuffer = reinterpret_cast<const char*>(buffer);
		const char* const endOfTextBuffer = reinterpret_cast<const char*>(buffer + bufferSize);

		GmPvListDBParser parser;
		parser.Parse(*this, startOfTextBuffer, endOfTextBuffer);
	}

	IO::StreamResult GmPvListDB::Write(IO::StreamWriter& writer)
	{
		std::pmr::monotonic_buffer_resource bufferResource {};
		std::pmr::vector<std::pmr::string> lines { &bufferResource };
		lines.reserve(Entries.size() * 60);

		char b[256];
		lines.emplace_back(b, sprintf_s(b, "pv_list.data_list.length=%zu\n", Entries.size()));
		for (size_t pvIndex = 0; pvIndex < Entries.size(); pvIndex++)
		{
			auto writeDifficultyArray = [&](const GmPvDifficultyEntries& entries, const char* name)
			{
				lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.length=%d\n", pvIndex, name, entries.Count));
				for (i32 i = 0; i < entries.Count; i++)
				{
					const auto& edition = entries.Editions[i];
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.ver=%d\n", pvIndex, name, i, edition.Version));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.st_year=%d\n", pvIndex, name, i, edition.StartDate.Year));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.st_month=%d\n", pvIndex, name, i, edition.StartDate.Month));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.st_day=%d\n", pvIndex, name, i, edition.StartDate.Day));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.edition=%d\n", pvIndex, name, i, edition.Edition));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.ed_year=%d\n", pvIndex, name, i, edition.EndDate.Year));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.ed_month=%d\n", pvIndex, name, i, edition.EndDate.Month));
					lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.%s.%d.ed_day=%d\n", pvIndex, name, i, edition.EndDate.Day));
				}
			};

			const auto& pvEntry = Entries[pvIndex];
			writeDifficultyArray(pvEntry.Easy, "easy");
			writeDifficultyArray(pvEntry.Normal, "normal");
			writeDifficultyArray(pvEntry.Hard, "hard");
			writeDifficultyArray(pvEntry.Extreme, "extreme");
			writeDifficultyArray(pvEntry.Encore, "encore");
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.name=", pvIndex)).append(pvEntry.Name).append("\n");
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.ignore=%d\n", pvIndex, pvEntry.Ignore));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.id=%d\n", pvIndex, pvEntry.ID));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_st_year=%d\n", pvIndex, pvEntry.AdvDemoStartDate.Year));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_st_month=%d\n", pvIndex, pvEntry.AdvDemoStartDate.Month));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_st_day=%d\n", pvIndex, pvEntry.AdvDemoStartDate.Day));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_ed_year=%d\n", pvIndex, pvEntry.AdvDemoEndDate.Year));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_ed_month=%d\n", pvIndex, pvEntry.AdvDemoEndDate.Month));
			lines.emplace_back(b, sprintf_s(b, "pv_list.%zu.adv_demo_ed_day=%d\n", pvIndex, pvEntry.AdvDemoEndDate.Day));
		}

		std::sort(lines.begin(), lines.end());

		for (const auto& line : lines)
			writer.WriteBuffer(line.data(), line.size());

		return IO::StreamResult::Success;
	}
}

#pragma once
#include "Types.h"
#include "Editor/Chart/Chart.h"
#include "IO/Stream/FileInterfaces.h"

namespace Comfy::Studio::Editor
{
	// NOTE: A separate file class has the advantage of making async saving (due to a copy) much easier
	//		 and better abstracts away the file format details
	class ComfyStudioChartFile : public IO::IStreamReadable, public IO::IStreamWritable, NonCopyable
	{
	public:
		// NOTE: Comfy Studio •ˆ–Ê (Fumen)
		static constexpr std::string_view Extension = ".csfm";
		static constexpr std::string_view FilterName = "Comfy Studio Chart";
		static constexpr std::string_view FilterSpec = "*.csfm";

	public:
		ComfyStudioChartFile() = default;
		ComfyStudioChartFile(const Chart& sourceChart);
		~ComfyStudioChartFile() = default;

	public:
		std::unique_ptr<Chart> MoveToChart();

	public:
		IO::StreamResult Read(IO::StreamReader& reader) override;
		IO::StreamResult Write(IO::StreamWriter& writer) override;

	private:
		void FromChart(const Chart& sourceChart);

	private:
		struct KeyValue { std::string Key, Value; };
		struct SmallKeyValueMap
		{
			// NOTE: A linear search should be more than fine for a small set of items
			std::vector<KeyValue> Items;
			inline void Add(std::string key, std::string value) { Items.push_back({ std::move(key), std::move(value) }); }
			inline std::string_view Find(std::string_view key) const { for (auto& item : Items) { if (item.Key == key) return item.Value; }; return ""; }
		};

		SmallKeyValueMap metadata;

		struct ChartData
		{
			struct ScaleData
			{
				std::vector<std::string> ButtonTypeNames;
				i32 TicksPerBeat;
				vec2 PlacementAreaSize;
				f32 FullAngleRotation;
			} Scale;

			struct TimeData
			{
				TimeSpan SongOffset;
				TimeSpan MovieOffset;
				TimeSpan Duration;
				TimeSpan SongPreviewStart;
				TimeSpan SongPreviewDuration;
			} Time;

			std::vector<TimelineTarget> Targets;
			std::vector<TempoChange> TempoChanges;

			struct ButtonSoundData
			{
				u32 ButtonID, SlideID, ChainSlideID, SliderTouchID;
			} ButtonSound;

			enum class DifficultyType : u8 { Easy, Normal, Hard, Extreme };
			enum class DifficultyVersion : u8 { Original, Extra };
			enum class DifficultyLevelWhole : u8 { Min = 0, Max = 10 };
			enum class DifficultyLevelFraction : u8 { Zero = 0, Half = 5 };
			struct DifficultyData
			{
				DifficultyType Type = DifficultyType::Hard;
				DifficultyVersion Version = DifficultyVersion::Original;
				DifficultyLevelWhole LevelWhole = static_cast<DifficultyLevelWhole>(7);
				DifficultyLevelFraction LevelFraction = DifficultyLevelFraction::Half;
			} Difficulty;
		} chart = {};
	};
}

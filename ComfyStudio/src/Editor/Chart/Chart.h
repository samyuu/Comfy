#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "SortedTargetList.h"
#include "SortedTempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class Chart
	{
	public:
		// NOTE: In case there is no audio file to take as a reference
		static constexpr auto FallbackDuration = TimeSpan::FromMinutes(1.0);
		static constexpr auto FallbackSongTitle = std::string_view { u8"É_É~Å[" };

	public:
		// NOTE: An empty path means the chart hasn't yet been saved to nor has been loaded from disk
		std::string ChartFilePath;

		// NOTE: Should (but doesn't have to) be relative to the chart file directory
		std::string SongFileName;

		struct PropertyData
		{
			struct SongInfo
			{
				std::string Title;
				std::string TitleReading;

				std::string Artist;
				std::string Album;

				std::string Lyricist;
				std::string Arranger;

				i32 TrackNumber;
				i32 DiskNumber;

				struct KeyValue { std::string Key, Value; };
				std::array<KeyValue, 4> ExtraInfo;
			} Song;

			struct CreatorInfo
			{
				std::string Name;
				std::string Comment;
			} Creator;

			struct SoundEffectInfo
			{
				std::string ButtonName;
				std::string SlideName;
				std::string ChainSlideFirstName;
				std::string ChainSlideSubName;
				std::string ChainSlideSuccessName;
				std::string ChainSlideFailureName;
				std::string SlideTouchName;
			} SoundEffect;

		} Properties;

		TimeSpan StartOffset = TimeSpan::Zero();
		TimeSpan Duration = TimeSpan::Zero();

		SortedTempoMap TempoMap;
		TimelineMap TimelineMap;

		SortedTargetList Targets;

	public:
		inline std::string_view SongTitleOrDefault() const { return Properties.Song.Title.empty() ? FallbackSongTitle : Properties.Song.Title; }
		inline TimeSpan DurationOrDefault() const { return (Duration <= TimeSpan::Zero()) ? FallbackDuration : Duration; }

		inline void UpdateMapTimes() { TimelineMap.CalculateMapTimes(TempoMap); }
	};
}

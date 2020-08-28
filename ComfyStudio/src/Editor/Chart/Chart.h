#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "SortedTargetList.h"
#include "SortedTempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	struct Chart
	{
	public:
		// NOTE: In case there is no audio file to take as a reference
		static constexpr auto FallbackDuration = TimeSpan::FromMinutes(1.0);
		static constexpr auto FallbackSongName = std::string_view { u8"–¼‘O‚Ì–³‚¢‰Ì" };

	public:
		std::string SongName = std::string(FallbackSongName);

		SortedTargetList Targets;
		SortedTempoMap TempoMap;
		TimelineMap TimelineMap;

		TimeSpan StartOffset = TimeSpan::Zero();
		TimeSpan Duration = FallbackDuration;

	public:
		inline void UpdateMapTimes() { TimelineMap.CalculateMapTimes(TempoMap); }
	};
}

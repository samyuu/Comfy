#pragma once
#include "SortedTargetList.h"
#include "SortedTempoMap.h"
#include "Timeline/TimelineMap.h"
#include "Time/TimeSpan.h"

namespace Comfy::Studio::Editor
{
	class Chart : NonCopyable
	{
	public:
		static constexpr TimeSpan FallbackDuration = TimeSpan::FromMinutes(1.0);

	public:
		Chart() = default;
		~Chart() = default;

	public:
		SortedTargetList& GetTargets();
		SortedTempoMap& GetTempoMap();
		TimelineMap& GetTimelineMap();

		TimeSpan GetStartOffset() const;
		void SetStartOffset(TimeSpan value);

		TimeSpan GetDuration() const;
		void SetDuration(TimeSpan value);

	private:
		SortedTargetList targets;
		SortedTempoMap tempoMap;
		TimelineMap timelineMap;

		TimeSpan startOffset = TimeSpan::Zero();
		TimeSpan duration = FallbackDuration;
	};
}

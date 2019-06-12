#pragma once
#include "TimelineBase.h"

namespace Editor
{
	class FrameTimeline : public TimelineBase
	{
	public:
		virtual float GetTimelinePosition(TimeSpan time) const override;
		virtual TimeSpan GetTimelineTime(float position) const override;

	protected:

		virtual void OnDrawTimlineDivisors() override;
		virtual void DrawTimelineCursor() override;
	};
}
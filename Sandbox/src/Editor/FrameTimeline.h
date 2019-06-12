#pragma once
#include "TimelineBase.h"

class FrameTimeline : public TimelineBase
{
public:
	virtual float GetTimelinePosition(TimeSpan time) const override;
	virtual TimeSpan GetTimelineTime(float position) const override;

protected:

	virtual void DrawTimlineDivisors() override;
	virtual void DrawTimelineCursor() override;
};
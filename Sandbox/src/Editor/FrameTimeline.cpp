#include "FrameTimeline.h"

float FrameTimeline::GetTimelinePosition(TimeSpan time) const
{
	return TimelineBase::GetTimelinePosition(time);
}

TimeSpan FrameTimeline::GetTimelineTime(float position) const
{
	return TimelineBase::GetTimelineTime(position);
}

void FrameTimeline::DrawTimlineDivisors()
{
}

void FrameTimeline::DrawTimelineCursor()
{
}

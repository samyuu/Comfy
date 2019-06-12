#include "AetTimeline.h"
#include "AetEditor.h"

namespace Editor
{
	bool AetTimeline::GetIsPlayback() const
	{
		return isPlayback;
	}

	float AetTimeline::GetTimelineSize() const
	{
		return 3939.0f;
	}
}
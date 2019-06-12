#pragma once
#include "../FrameTimeline.h"

namespace Editor
{
	class AetEditor;

	class AetTimeline : public FrameTimeline
	{
	public:
		
	private:
		bool isPlayback = false;

		virtual bool GetIsPlayback() const override;
		virtual float GetTimelineSize() const override;
	};
}
#pragma once
#include "TimelineTick.h"
#include "../../ImGui/imgui.h"

namespace Editor
{
	class Cursor
	{
	public:
		const float HEAD_WIDTH = 17.0f;
		const float HEAD_HEIGHT = 8.0f;

		TimelineTick Tick;
		TimelineTick TickOnPlaybackStart;
	};
}
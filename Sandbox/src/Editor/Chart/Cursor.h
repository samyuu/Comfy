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

		inline ImU32 GetColor() { return color; };

	private:
		ImU32 color = ImColor(0.71f, 0.54f, 0.15f);;
	};
}
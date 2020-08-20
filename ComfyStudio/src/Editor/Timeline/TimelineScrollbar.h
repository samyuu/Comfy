#pragma once
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TimelineScrollbar
	{
	public:
		TimelineScrollbar(ImGuiAxis axis, vec2 timelineScrollbarSize);
		TimelineScrollbar() = default;

	public:
		bool Gui(float& inOutScroll, float availableScroll, float maxScroll, ImRect scrollbarRegion);

	private:
		ImGuiAxis axis;
		vec2 timelineScrollbarSize;

		float clickDeltaToGrabCenter = 0.0f;
		bool held = false, hovered = false;
	};
}

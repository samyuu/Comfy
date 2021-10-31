#pragma once
#include "Types.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TimelineScrollbar
	{
	public:
		TimelineScrollbar(ImGuiAxis axis, vec2 timelineScrollbarSize);
		~TimelineScrollbar() = default;

	public:
		bool Gui(f32& inOutScroll, f32 availableScroll, f32 maxScroll, ImRect scrollbarRegion);

	private:
		ImGuiAxis axis;
		vec2 timelineScrollbarSize;

		f32 clickDeltaToGrabCenter = 0.0f;
		bool held = false, hovered = false;
	};
}

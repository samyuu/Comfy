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
		void Gui(float scroll, float maxScroll, ImRect scrollbarRegion);

	private:
		ImU32 GetGrabColor() const;

	private:
		ImGuiAxis axis;
		const vec2 timelineScrollbarSize;

		// TODO:
		bool held = false, hovered = false;
	};
}

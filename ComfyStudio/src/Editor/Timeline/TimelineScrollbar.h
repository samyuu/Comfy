#pragma once
#include "ImGui/Gui.h"

namespace Editor
{
	class TimelineScrollbar
	{
	public:
		TimelineScrollbar(ImGuiAxis axis, const vec2 timelineScrollbarSize);

		void DrawGui(const float scroll, const float maxScroll, const ImRect scrollbarRegion);

	private:
		ImGuiAxis axis;
		const vec2 timelineScrollbarSize;
		
		// TODO:
		bool held = false, hovered = false;
	
		ImU32 GetGrabColor() const;
	};
}
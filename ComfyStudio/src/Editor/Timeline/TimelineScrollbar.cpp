#include "TimelineScrollbar.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	TimelineScrollbar::TimelineScrollbar(ImGuiAxis axis, vec2 timelineScrollbarSize)
		: axis(axis), timelineScrollbarSize(timelineScrollbarSize)
	{
		assert(axis == ImGuiAxis_X || axis == ImGuiAxis_Y);
	}

	bool TimelineScrollbar::Gui(float& inOutScroll, float maxScroll, ImRect scrollbarRegion)
	{
		auto drawList = Gui::GetWindowDrawList();
		const auto& style = Gui::GetStyle();

		const vec2 scrollbarRegionSize = scrollbarRegion.GetSize();
		const float scrollbarAxisSize = scrollbarRegionSize[axis];

		const float visibleContentPercentage = (maxScroll <= 0.0f) ? 0.0f : (scrollbarAxisSize / (maxScroll + scrollbarAxisSize));
		const float visibleContentSize = glm::clamp(((maxScroll <= 0.0f) ? scrollbarAxisSize : visibleContentPercentage * scrollbarAxisSize), (style.GrabMinSize * 2.0f), scrollbarAxisSize);

		const float avilableSize = scrollbarAxisSize - visibleContentSize;
		const float grabSizePercentage = (maxScroll <= 0.0f) ? 0.0f : (inOutScroll / maxScroll);

		const float grabPosition = avilableSize * grabSizePercentage;

		ImRect scrollbarGrabRegion;
		if (axis == ImGuiAxis_X)
		{
			constexpr vec2 grabPadding = vec2(2.0f, 3.0f);
			scrollbarGrabRegion = ImRect(
				scrollbarRegion.Min + vec2(+grabPadding.x + grabPosition, grabPadding.y),
				scrollbarRegion.Min + vec2(-grabPadding.x + grabPosition + visibleContentSize, timelineScrollbarSize.y - grabPadding.y));
		}
		else if (axis == ImGuiAxis_Y)
		{
			constexpr vec2 grabPadding = vec2(3.0f, 2.0f);
			scrollbarGrabRegion = ImRect(
				scrollbarRegion.Min + vec2(+grabPadding.x + 0.0, grabPadding.y + grabPosition),
				scrollbarRegion.Min + vec2(-grabPadding.x + timelineScrollbarSize.x, -grabPadding.y + grabPosition + visibleContentSize));
		}

		const vec2 scrollbarGrabRegionSize = scrollbarGrabRegion.GetSize();
		if (scrollbarGrabRegionSize.x < 0.0f || scrollbarGrabRegionSize.y < 0.0f)
			return false;

		drawList->AddRectFilled(scrollbarGrabRegion.Min, scrollbarGrabRegion.Max, GetGrabColor(), style.ScrollbarRounding);

		return held;
	}
	}

	ImU32 TimelineScrollbar::GetGrabColor() const
	{
		return Gui::GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
	}
}

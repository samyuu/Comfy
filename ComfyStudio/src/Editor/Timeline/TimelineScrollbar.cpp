#include "Core/CoreTypes.h"
#include "TimelineScrollbar.h"

namespace Editor
{
	TimelineScrollbar::TimelineScrollbar(ImGuiAxis axis, const vec2 timelineScrollbarSize)
		: axis(axis), timelineScrollbarSize(timelineScrollbarSize)
	{
		assert(axis == ImGuiAxis_X || axis == ImGuiAxis_Y);
	}

	void TimelineScrollbar::DrawGui(const float scroll, const float maxScroll, const ImRect scrollbarRegion)
	{
		auto drawList = Gui::GetWindowDrawList();
		const auto& style = Gui::GetStyle();

		const vec2 scrollbarRegionSize = scrollbarRegion.GetSize();
		const float scrollbarAxisSize = scrollbarRegionSize[axis];

		const float visibleContentPercentage = (maxScroll <= 0.0f) ? 0.0f : (scrollbarAxisSize / maxScroll);
		const float visibleContentSize = glm::clamp(((maxScroll <= 0.0f) ? scrollbarAxisSize : visibleContentPercentage * scrollbarAxisSize), (style.GrabMinSize * 2.0f), scrollbarAxisSize);
		const float avilableSize = scrollbarAxisSize - visibleContentSize;
		const float grabSizePercentage = (maxScroll <= 0.0f) ? 0.0f : (scroll / maxScroll);

		ImRect scrollbarGrabRegion;

		if (axis == ImGuiAxis_X)
		{
			const float grabX = avilableSize * grabSizePercentage;

			constexpr vec2 grabPadding = vec2(2.0f, 3.0f);
			scrollbarGrabRegion = ImRect(
				scrollbarRegion.Min + vec2(+grabPadding.x + grabX, grabPadding.y),
				scrollbarRegion.Min + vec2(-grabPadding.x + grabX + visibleContentSize, timelineScrollbarSize.y - grabPadding.y));
		}
		else if (axis == ImGuiAxis_Y)
		{
			const float grabY = avilableSize * grabSizePercentage;

			constexpr vec2 grabPadding = vec2(3.0f, 2.0f);
			scrollbarGrabRegion = ImRect(
				scrollbarRegion.Min + vec2(+grabPadding.x + 0.0, grabPadding.y + grabY),
				scrollbarRegion.Min + vec2(-grabPadding.x + timelineScrollbarSize.x, -grabPadding.y + grabY + visibleContentSize));
		}

		drawList->AddRectFilled(scrollbarGrabRegion.Min, scrollbarGrabRegion.Max, GetGrabColor(), style.ScrollbarRounding);
	}

	ImU32 TimelineScrollbar::GetGrabColor() const
	{
		return Gui::GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
	}
}
#include "TimelineScrollbar.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	TimelineScrollbar::TimelineScrollbar(ImGuiAxis axis, vec2 timelineScrollbarSize)
		: axis(axis), timelineScrollbarSize(timelineScrollbarSize)
	{
		assert(axis == ImGuiAxis_X || axis == ImGuiAxis_Y);
	}

	bool TimelineScrollbar::Gui(f32& inOutScroll, f32 availableScroll, f32 maxScroll, ImRect scrollbarRegion)
	{
		if (scrollbarRegion.GetWidth() <= 0.0f || scrollbarRegion.GetHeight() <= 0.0f)
			return false;

		// NOTE: Based on ImGui::Scrollbar() but without the window specific logic
		auto* window = GImGui->CurrentWindow;
		const auto& style = GImGui->Style;

		const auto id = Gui::GetWindowScrollbarID(window, axis);
		Gui::KeepAliveID(id);

		const auto cornderFlags = (axis == ImGuiAxis_X) ?
			(ImDrawCornerFlags_BotLeft | ImDrawCornerFlags_BotRight) :
			(ImDrawCornerFlags_TopRight | ImDrawCornerFlags_BotRight);

		window->DrawList->AddRectFilled(scrollbarRegion.Min, scrollbarRegion.Max, Gui::GetColorU32(ImGuiCol_ScrollbarBg), window->WindowRounding, cornderFlags);

		scrollbarRegion.Expand(ImVec2(
			-ImClamp(static_cast<f32>(static_cast<i32>((scrollbarRegion.Max.x - scrollbarRegion.Min.x - 2.0f)) * 0.5f), 0.0f, 3.0f),
			-ImClamp(static_cast<f32>(static_cast<i32>((scrollbarRegion.Max.y - scrollbarRegion.Min.y - 2.0f)) * 0.5f), 0.0f, 3.0f))
		);

		const f32 axisScrollbarSize = scrollbarRegion.GetSize()[axis];
		const f32 axisWinSize = ImMax(ImMax(maxScroll, availableScroll), 1.0f);
		const f32 grabHeightpixels = ImClamp(axisScrollbarSize * (availableScroll / axisWinSize), style.GrabMinSize, axisScrollbarSize);
		const f32 grabHeightNorm = grabHeightpixels / axisScrollbarSize;

		Gui::ButtonBehavior(scrollbarRegion, id, &hovered, &held, ImGuiButtonFlags_NoNavFocus);

		const f32 scrollMax = ImMax(1.0f, maxScroll - availableScroll);
		f32 scrollRatio = ImSaturate(inOutScroll / scrollMax);
		f32 axisGrabNorm = scrollRatio * (axisScrollbarSize - grabHeightpixels) / axisScrollbarSize;

		if (held && grabHeightNorm < 1.0f)
		{
			const f32 scrollbarAxisPos = scrollbarRegion.Min[axis];
			const f32 mouseAxisPos = GImGui->IO.MousePos[axis];

			const f32 axisClickedNorm = ImSaturate((mouseAxisPos - scrollbarAxisPos) / axisScrollbarSize);
			Gui::SetHoveredID(id);

			bool seekAbsolute = false;
			if (const bool previouslyHeld = (GImGui->ActiveId == id); !previouslyHeld)
			{
				if (axisClickedNorm >= axisGrabNorm && axisClickedNorm <= axisGrabNorm + grabHeightNorm)
				{
					clickDeltaToGrabCenter = axisClickedNorm - axisGrabNorm - grabHeightNorm * 0.5f;
				}
				else
				{
					seekAbsolute = true;
					clickDeltaToGrabCenter = 0.0f;
				}
			}

			const f32 scroll_v_norm = ImSaturate((axisClickedNorm - clickDeltaToGrabCenter - grabHeightNorm * 0.5f) / (1.0f - grabHeightNorm));
			inOutScroll = static_cast<f32>(static_cast<i32>(0.5f + scroll_v_norm * scrollMax));

			scrollRatio = ImSaturate(inOutScroll / scrollMax);
			axisGrabNorm = scrollRatio * (axisScrollbarSize - grabHeightpixels) / axisScrollbarSize;

			if (seekAbsolute)
				clickDeltaToGrabCenter = axisClickedNorm - axisGrabNorm - grabHeightNorm * 0.5f;
		}

		const auto grabColor = Gui::GetColorU32(held ? ImGuiCol_ScrollbarGrabActive : hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab);
		const auto grabRect = (axis == ImGuiAxis_X) ?
			ImRect(
				ImLerp(scrollbarRegion.Min.x, scrollbarRegion.Max.x, axisGrabNorm),
				scrollbarRegion.Min.y,
				ImMin(ImLerp(scrollbarRegion.Min.x, scrollbarRegion.Max.x, axisGrabNorm) + grabHeightpixels, scrollbarRegion.Max.x),
				scrollbarRegion.Max.y) :
			ImRect(
				scrollbarRegion.Min.x,
				ImLerp(scrollbarRegion.Min.y, scrollbarRegion.Max.y, axisGrabNorm),
				scrollbarRegion.Max.x,
				ImMin(ImLerp(scrollbarRegion.Min.y, scrollbarRegion.Max.y, axisGrabNorm) + grabHeightpixels, scrollbarRegion.Max.y));

		window->DrawList->AddRectFilled(grabRect.Min, grabRect.Max, grabColor, style.ScrollbarRounding);
		return held;
	}
}

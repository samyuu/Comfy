#include "PlayTestWindow.h"

namespace Comfy::Studio::Editor
{
	PlayTestWindow::PlayTestWindow(PlayTestSharedContext sharedContext) : sharedContext(sharedContext)
	{
		context.RenderTarget->Param.ClearColor = vec4(0.14f, 0.14f, 0.14f, 1.0f);
		context.RenderTarget->Param.PostProcessingEnabled = true;
	}

	void PlayTestWindow::ExclusiveGui()
	{
		windowRect = Gui::GetCurrentWindow()->Rect();
		renderRegionRect = Gui::FitFixedAspectRatioImage(windowRect, Rules::PlacementAreaSize);

		context.Camera.ProjectionSize = renderRegionRect.GetSize();
		context.Camera.CenterAndZoomToFit(Rules::PlacementAreaSize);
		context.RenderTarget->Param.Resolution = vec2(renderRegionRect.GetSize());

		sharedContext.Renderer->Begin(context.Camera, *context.RenderTarget);
		core.UpdateTick();
		sharedContext.Renderer->End();

		auto* windowDrawList = Gui::GetWindowDrawList();

		if (renderRegionRect != windowRect)
		{
			if (!windowBackgroundCheckerboard.has_value())
				windowBackgroundCheckerboard.emplace(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.14f, 0.14f, 0.14f, 1.0f), 8);

			windowBackgroundCheckerboard->AddToDrawList(windowDrawList, windowRect);
		}

		windowDrawList->AddImage(context.RenderTarget->GetTextureID(), renderRegionRect.GetTL(), renderRegionRect.GetBR());
		core.OverlayGui();
	}

	bool PlayTestWindow::ExitRequestedThisFrame()
	{
		return core.ExitRequestedThisFrame();
	}

	void PlayTestWindow::SetWorkingChart(Chart* chart)
	{
		sharedContext.Chart = chart;
	}

	vec2 PlayTestWindow::WorldToScreenSpace(const vec2 worldSpace) const
	{
		const auto scale = (renderRegionRect.GetSize() / Rules::PlacementAreaSize);
		return renderRegionRect.GetTL() + (worldSpace * scale);
	}

	vec2 PlayTestWindow::ScreenToWorldSpace(const vec2 screenSpace) const
	{
		const auto scale = (Rules::PlacementAreaSize / renderRegionRect.GetSize());
		return (screenSpace - renderRegionRect.GetTL()) * scale;
	}
}

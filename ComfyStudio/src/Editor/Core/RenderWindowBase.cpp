#include "RenderWindowBase.h"
#include "Theme.h"

namespace Editor
{
	void RenderWindowBase::Initialize()
	{
		renderTarget.Initialize(RenderTargetDefaultSize);
		renderRegion = lastRenderRegion = ImRect(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

		OnInitialize();
	}

	void RenderWindowBase::DrawGui()
	{
		RenderWindowBase::PopWindowPadding();
		OnDrawGui();

		Gui::PushID(this);
		Gui::BeginChild("BaseChild##RenderWindowBase", vec2(0.0f, 0.0f), false, GetChildWinodwFlags());

		lastRenderRegion = renderRegion;

		renderRegion.Min = Gui::GetWindowPos();
		renderRegion.Max = renderRegion.Min + Gui::GetWindowSize();

		const ImRect fullRenderRegion = renderRegion;

		if (GetKeepAspectRatio())
		{
			vec2 renderRegionSize = renderRegion.GetSize();
			const float renderRegionAspectRatio = renderRegionSize.x / renderRegionSize.y;

			if (renderRegionAspectRatio <= targetAspectRatio)
			{
				// NOTE: Output is taller than it is wider, bars on top / bottom
				float presentHeight = glm::round((renderRegionSize.x / targetAspectRatio) + 0.5f);
				float barHeight = glm::round((renderRegionSize.y - presentHeight) / 2.0f);

				renderRegion.Min.y += barHeight;
				renderRegion.Max.y += barHeight;
				renderRegion.Max.y = renderRegion.Min.y + presentHeight;
			}
			else
			{
				// NOTE: Output is wider than it is tall, bars left / right
				int presentWidth = static_cast<int>((renderRegionSize.y * targetAspectRatio) + 0.5f);
				int barWidth = static_cast<int>((renderRegionSize.x - presentWidth) / 2.0f);

				renderRegion.Min.x += barWidth;
				renderRegion.Max.x += barWidth;
				renderRegion.Max.x = renderRegion.Min.x + presentWidth;
			}
		}

		const vec2 renderSize = renderRegion.GetSize(), lastRenderSize = lastRenderRegion.GetSize();
		wasResized = (renderSize.x != lastRenderSize.x) || (renderSize.y != lastRenderSize.y);
		if (wasResized)
			needsResizing = true;

		ImGuiWindow* currentWindow = Gui::GetCurrentWindow();

		if (!currentWindow->Hidden)
		{
			if (needsResizing && Gui::GetFrameCount() >= 2)
				OnResize(ivec2(renderSize.x, renderSize.y));

			OnUpdate();
			if (Gui::IsWindowFocused())
				OnUpdateInput();
			OnRender();
		}

		if (GetKeepAspectRatio())
		{
			currentWindow->DrawList->AddRectFilled(
				fullRenderRegion.Min, 
				fullRenderRegion.Max, 
				Gui::GetColorU32(ImGuiCol_WindowBg));
		}

		currentWindow->DrawList->AddImage(
			renderTarget.GetTexture().GetVoidTexture(),
			renderRegion.GetTL(),
			renderRegion.GetBR(),
			Gui::UV0_GL, Gui::UV1_GL);

		PostDrawGui();

		Gui::EndChild();
		Gui::PopID();

		RenderWindowBase::PushWindowPadding();
	}

	vec2 RenderWindowBase::GetRelativeMouse() const
	{
		return Gui::GetMousePos() - renderRegion.Min;
	}

	const ImRect& RenderWindowBase::GetRenderRegion() const
	{
		return renderRegion;
	}

	void RenderWindowBase::OnResize(ivec2 size)
	{
		renderTarget.Resize(size);
		needsResizing = false;
	}
}
#include "RenderWindowBase.h"
#include "Theme.h"

namespace Editor
{
	void RenderWindowBase::Initialize()
	{
		renderTarget.Initialize(RENDER_TARGET_DEFAULT_WIDTH, RENDER_TARGET_DEFAULT_WIDTH);
		renderRegion = lastRenderRegion = ImRect(0, 0, renderTarget.GetWidth(), renderTarget.GetHeight());

		OnInitialize();
	}

	void RenderWindowBase::DrawGui()
	{
		RenderWindowBase::PopWindowPadding();
		OnDrawGui();

		ImGui::PushID((void*)this);
		ImGui::BeginChild("RenderWindowBaseChild");

		lastRenderRegion = renderRegion;

		renderRegion.Min = ImGui::GetWindowPos();
		renderRegion.Max = renderRegion.Min + ImGui::GetWindowSize();

		if (GetKeepAspectRatio())
		{
			ImVec2 renderRegionSize = renderRegion.GetSize();
			const float outputAspect = renderRegionSize.x / renderRegionSize.y;

			if (outputAspect <= targetAspectRatio)
			{
				// output is taller than it is wider, bars on top/bottom
				float presentHeight = (renderRegionSize.x / targetAspectRatio) + 0.5f;
				float barHeight = (renderRegionSize.y - presentHeight) / 2;

				renderRegion.Min.y += barHeight;
				renderRegion.Max.y += barHeight;
				renderRegion.Max.y = renderRegion.Min.y + presentHeight;
			}
			else
			{
				// output is wider than it is tall, bars left/right
				int presentWidth = (renderRegionSize.y * targetAspectRatio) + 0.5f;
				int barWidth = (renderRegionSize.x - presentWidth) / 2;

				renderRegion.Min.x += barWidth;
				renderRegion.Max.x += barWidth;
				renderRegion.Max.x = renderRegion.Min.x + presentWidth;
			}
		}

		const ImVec2 renderSize = renderRegion.GetSize(), lastRenderSize = lastRenderRegion.GetSize();
		wasResized = (renderSize.x != lastRenderSize.x) || (renderSize.y != lastRenderSize.y);

		if (GetWasResized() && ImGui::GetFrameCount() >= 2)
			OnResize(renderSize.x, renderSize.y);

		if (!ImGui::GetCurrentWindow()->Hidden)
		{
			OnUpdate();
			if (ImGui::IsWindowFocused())
				OnUpdateInput();
			OnRender();
		}

		ImGui::GetWindowDrawList()->AddImage(
			renderTarget.GetTexture().GetVoidTexture(),
			renderRegion.GetTL(),
			renderRegion.GetBR(),
			ImGui::UV0_GL, ImGui::UV1_GL);

		ImGui::EndChild();
		ImGui::PopID();

		RenderWindowBase::PushWindowPadding();
	}

	const ImRect& RenderWindowBase::GetRenderRegion() const
	{
		return renderRegion;
	}

	void RenderWindowBase::OnResize(int width, int height)
	{
		renderTarget.Resize(width, height);
	}
}
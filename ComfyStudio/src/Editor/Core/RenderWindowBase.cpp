#include "RenderWindowBase.h"
#include "Theme.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	void RenderWindowBase::Initialize()
	{
		if (auto externalRenderTarget = GetExternalRenderTarget(); externalRenderTarget == nullptr)
		{
			owningRenderTarget = GetShouldCreateDepthRenderTarget() ?
				std::make_unique<D3D11::DepthRenderTarget>(D3D11::RenderTargetDefaultSize, DXGI_FORMAT_D32_FLOAT) :
				std::make_unique<D3D11::RenderTarget>(D3D11::RenderTargetDefaultSize);
			
			renderRegion = lastRenderRegion = ImRect(vec2(0.0f, 0.0f), vec2(owningRenderTarget->GetSize()));
		}
		else
		{
			renderRegion = lastRenderRegion = ImRect(vec2(0.0f, 0.0f), vec2(externalRenderTarget->GetSize()));
		}

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
				OnResize(renderSize);

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

		GPU_RenderTarget* externalRenderTarget = GetExternalRenderTarget();
		GPU_RenderTarget* outputRenderTarget = (externalRenderTarget != nullptr) ? externalRenderTarget : owningRenderTarget.get();

		if (outputRenderTarget != nullptr)
		{
			currentWindow->DrawList->AddImage(
				*outputRenderTarget,
				renderRegion.GetTL(),
				renderRegion.GetBR());
		}

		PostDrawGui();

		Gui::EndChild();
		Gui::PopID();

		RenderWindowBase::PushWindowPadding();
	}

	vec2 RenderWindowBase::GetRelativeMouse() const
	{
		return Gui::GetMousePos() - renderRegion.Min;
	}

	void RenderWindowBase::OnResize(ivec2 size)
	{
		if (resizeOwningRenderTarget && owningRenderTarget != nullptr)
			owningRenderTarget->Resize(size);

		needsResizing = false;
	}
}

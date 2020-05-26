#include "RenderWindow.h"

namespace Comfy
{
	void RenderWindow::BeginEndGui(const char* windowName, bool* isOpen, ImGuiWindowFlags flags)
	{
		constexpr vec2 windowPadding = vec2(2.0f, 2.0f);

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
		PreBeginWindow();
		if (Gui::Begin(windowName, isOpen, flags))
		{
			Gui::PopStyleVar();

			CheckUpdateOnFirstFrame();
			PreRenderTextureGui();
			Gui::BeginChild("RenderTextureChild##RenderWindow", vec2(0.0f, 0.0f), false, GetRenderTextureChildWindowFlags());
			RenderTextureGui();
			PostRenderTextureGui();
			Gui::EndChild();

			Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, windowPadding);
		}
		Gui::End();
		Gui::PopStyleVar();
	}

	void RenderWindow::BeginEndGuiFullScreen(const char* windowName)
	{
		constexpr ImGuiWindowFlags fullscreenWindowFlags =
			ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoNavFocus |
			ImGuiWindowFlags_NoSavedSettings;

		const ImGuiViewport* viewport = Gui::GetMainViewport();
		Gui::SetNextWindowPos(viewport->Pos);
		Gui::SetNextWindowSize(viewport->Size);
		Gui::SetNextWindowViewport(viewport->ID);

		Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(0.0f, 0.0f));
		Gui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

		if (Gui::Begin(windowName, nullptr, fullscreenWindowFlags))
		{
			Gui::PushID(this);

			CheckUpdateOnFirstFrame();
			RenderTextureGui();

			Gui::PopID();
		}
		Gui::End();
		Gui::PopStyleVar(2);
	}

	vec2 RenderWindow::GetRelativeMouse() const
	{
		return Gui::GetMousePos() - renderRegion.Min;
	}

	bool RenderWindow::GetKeepAspectRatio() const
	{
		return keepAspectRatio;
	}

	void RenderWindow::SetKeepAspectRatio(bool value)
	{
		keepAspectRatio = value;
	}

	float RenderWindow::GetTargetAspectRatio() const
	{
		return targetAspectRatio;
	}

	void RenderWindow::SetTargetAspectRatio(float value)
	{
		targetAspectRatio = value;
	}

	bool RenderWindow::GetWasResized() const
	{
		return wasResized;
	}

	ImRect RenderWindow::GetRenderRegion() const
	{
		return renderRegion;
	}

	void RenderWindow::CheckUpdateOnFirstFrame()
	{
		if (!isFirstFrame)
			return;

		OnFirstFrame();
		isFirstFrame = false;
	}

	void RenderWindow::RenderTextureGui()
	{
		UpdateRenderRegion();
		AdjustSizeToTargetAspectRatio();

		const auto renderRegionSize = ivec2(vec2(renderRegion.GetSize()));
		const auto lastRenderRegionSize = ivec2(vec2(lastRenderRegion.GetSize()));

		if (wasResized = (renderRegionSize != lastRenderRegionSize))
			needsResizing = true;

		ImGuiWindow* currentWindow = Gui::GetCurrentWindow();

		if (!currentWindow->Hidden)
		{
			// NOTE: Skip first few ImGui frames to avoid needlessly resizing while the layout config is still being applied
			if (needsResizing && Gui::GetFrameCount() >= 2)
			{
				OnResize(renderRegionSize);
				needsResizing = false;
			}

			OnRender();
		}

		if (keepAspectRatio)
		{
			currentWindow->DrawList->AddRectFilled(
				fullRenderRegion.Min,
				fullRenderRegion.Max,
				Gui::GetColorU32(ImGuiCol_WindowBg));
		}

		if (const auto textureID = GetTextureID(); textureID != nullptr)
		{
			currentWindow->DrawList->AddImage(
				textureID,
				renderRegion.GetTL(),
				renderRegion.GetBR());
		}
	}

	void RenderWindow::UpdateRenderRegion()
	{
		lastRenderRegion = renderRegion;

		renderRegion.Min = Gui::GetWindowPos();
		renderRegion.Max = renderRegion.Min + Gui::GetWindowSize();

		fullRenderRegion = renderRegion;
	}

	void RenderWindow::AdjustSizeToTargetAspectRatio()
	{
		if (!keepAspectRatio)
			return;

		const auto renderRegionSize = vec2(renderRegion.GetSize());
		const auto renderRegionAspectRatio = renderRegionSize.x / renderRegionSize.y;

		if (renderRegionAspectRatio <= targetAspectRatio) // NOTE: Taller than wide, bars on top / bottom
		{
			const auto presentHeight = glm::round((renderRegionSize.x / targetAspectRatio) + 0.5f);
			const auto barHeight = glm::round((renderRegionSize.y - presentHeight) / 2.0f);

			renderRegion.Min.y += barHeight;
			renderRegion.Max.y += barHeight;
			renderRegion.Max.y = renderRegion.Min.y + presentHeight;
		}
		else // NOTE: Wider than tall, bars on left / right
		{
			const auto presentWidth = static_cast<int>((renderRegionSize.y * targetAspectRatio) + 0.5f);
			const auto barWidth = static_cast<int>((renderRegionSize.x - presentWidth) / 2.0f);

			renderRegion.Min.x += barWidth;
			renderRegion.Max.x += barWidth;
			renderRegion.Max.x = renderRegion.Min.x + presentWidth;
		}
	}
}

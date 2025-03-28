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

	bool RenderWindow::GetWindowBackgroundCheckerboardEnabled() const
	{
		return windowBackgroundCheckerboardEnabled;
	}

	void RenderWindow::SetWindowBackgroundCheckerboardEnabled(bool value)
	{
		windowBackgroundCheckerboardEnabled = value;
	}

	bool RenderWindow::GetRenderBackgroundCheckerboardEnabled() const
	{
		return renderBackgroundCheckerboardEnabled;
	}

	void RenderWindow::SetRenderBackgroundCheckerboardEnabled(bool value)
	{
		renderBackgroundCheckerboardEnabled = value;
	}

	bool RenderWindow::GetWasResized() const
	{
		return wasResized;
	}

	ImRect RenderWindow::GetRenderRegion() const
	{
		return renderRegion;
	}

	void RenderWindow::RenderTextureGui()
	{
		UpdateRenderRegion();
		
		if (keepAspectRatio)
			renderRegion = Gui::FitFixedAspectRatio(renderRegion, targetAspectRatio);

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
			if (windowBackgroundCheckerboardEnabled)
			{
				if (!windowBackgroundCheckerboard.has_value())
					windowBackgroundCheckerboard.emplace(vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4(0.14f, 0.14f, 0.14f, 1.0f), 8);

				windowBackgroundCheckerboard->AddToDrawList(currentWindow->DrawList, fullRenderRegion);
			}
			else
			{
				currentWindow->DrawList->AddRectFilled(
					fullRenderRegion.Min,
					fullRenderRegion.Max,
					Gui::GetColorU32(ImGuiCol_WindowBg));
			}
		}

		if (renderBackgroundCheckerboardEnabled)
		{
			if (!renderBackgroundCheckerboard.has_value())
				renderBackgroundCheckerboard.emplace(vec4(0.18f, 0.18f, 0.18f, 1.0f), vec4(0.15f, 0.15f, 0.15f, 1.0f), 8);

			renderBackgroundCheckerboard->AddToDrawList(currentWindow->DrawList, renderRegion);
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
}

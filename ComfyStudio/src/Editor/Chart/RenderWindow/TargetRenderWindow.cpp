#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager)
		: chartEditor(parent), timeline(timeline), undoManager(undoManager)
	{
		workingChart = chartEditor.GetChart();

		SetWindowBackgroundCheckerboardEnabled(true);

		SetKeepAspectRatio(true);
		SetTargetAspectRatio(renderSize.x / renderSize.y);

		backgroundCheckerboard.Color = vec4(0.20f, 0.20f, 0.20f, 1.0f);
		backgroundCheckerboard.ColorAlt = vec4(0.26f, 0.26f, 0.26f, 1.0f);

		renderHelper = std::make_unique<TargetRenderHelper>();

		// TODO: Should maybe me owned by the ChartEditor instead (?)
		renderer = std::make_unique<Render::Renderer2D>();
		renderHelper->SetAetSprGetter(*renderer);

		renderTarget = Render::Renderer2D::CreateRenderTarget();
	}

	ImTextureID TargetRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
	}

	ImGuiWindowFlags TargetRenderWindow::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void TargetRenderWindow::PreRenderTextureGui()
	{
	}

	void TargetRenderWindow::PostRenderTextureGui()
	{
	}

	void TargetRenderWindow::OnResize(ivec2 newSize)
	{
		renderTarget->Param.Resolution = newSize;
		renderTarget->Param.ClearColor = GetColorVec4(EditorColor_DarkClear);

		camera.ProjectionSize = vec2(newSize);
		camera.Position = vec2(0.0f, 0.0f);
		camera.Zoom = camera.ProjectionSize.x / renderSize.x;
	}

	void TargetRenderWindow::OnRender()
	{
		renderHelper->UpdateAsyncLoading();

		renderer->Begin(camera, *renderTarget);
		{
			RenderBackground();
			RenderHUD();
		}
		renderer->End();
	}

	void TargetRenderWindow::RenderBackground()
	{
		backgroundCheckerboard.Size = renderSize;
		backgroundCheckerboard.Render(*renderer);

		renderer->Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), renderSize, vec4(0.0f, 0.0f, 0.0f, backgroundDim)));
	}

	void TargetRenderWindow::RenderHUD()
	{
		TargetRenderHelper::HUD hud;
		hud.SongName = workingChart->SongName;
		hud.IsPlayback = chartEditor.GetIsPlayback();
		hud.PlaybackTime = chartEditor.GetPlaybackTimeAsync();
		hud.PlaybackTimeOnStart = chartEditor.GetPlaybackTimeOnPlaybackStart();
		hud.Duration = workingChart->Duration;

		renderHelper->DrawHUD(*renderer, hud);
	}
}

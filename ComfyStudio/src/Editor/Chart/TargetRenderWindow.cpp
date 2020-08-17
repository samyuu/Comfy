#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"
#include "Editor/Chart/ChartEditor.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
		workingChart = chartEditor.GetChart();
	}

	ImTextureID TargetRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
	}

	ImGuiWindowFlags TargetRenderWindow::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void TargetRenderWindow::OnFirstFrame()
	{
		SetWindowBackgroundCheckerboardEnabled(true);

		SetKeepAspectRatio(true);
		SetTargetAspectRatio(renderSize.x / renderSize.y);

		backgroundCheckerboard.Color = vec4(0.20f, 0.20f, 0.20f, 1.0f);
		backgroundCheckerboard.ColorAlt = vec4(0.26f, 0.26f, 0.26f, 1.0f);

		renderer = std::make_unique<Render::Renderer2D>();
		renderTarget = Render::Renderer2D::CreateRenderTarget();

		sprSetLoadFuture = IO::File::LoadAsync<SprSet>(sprSetFilePath);
		aetSetLoadFuture = IO::File::LoadAsync<AetSet>(aetSetFilePath);
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
		if (loadingContent)
			UpdateContentLoading();

		renderer->Begin(camera, *renderTarget);
		{
			RenderBackground();

			if (!loadingContent)
				RenderTestAet();
		}
		renderer->End();
	}

	void TargetRenderWindow::RenderBackground()
	{
		backgroundCheckerboard.Size = renderSize;
		backgroundCheckerboard.Render(*renderer);

		renderer->Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), renderSize, vec4(0.0f, 0.0f, 0.0f, backgroundDim)));
	}

	void TargetRenderWindow::RenderTestAet()
	{
		auto tryDrawLayer = [&](const auto& layer, frame_t frame)
		{
			if (layer != nullptr)
				renderer->Aet().DrawLayer(*layer, frame);
		};

		tryDrawLayer(layerCache.FrameUp, 0.0f);
		tryDrawLayer(layerCache.FrameBottom, 0.0f);
		tryDrawLayer(layerCache.LifeGauge, 0.0f);
		tryDrawLayer(layerCache.SongEnergyBase, 100.0f);
		tryDrawLayer(layerCache.SongIconLoop, 0.0f);
		tryDrawLayer(layerCache.LevelInfoHard, 0.0f);
		tryDrawLayer(layerCache.SongIconLoop, 0.0f);
	}

	void TargetRenderWindow::UpdateContentLoading()
	{
		if (sprSet != nullptr && aetSet != nullptr)
			loadingContent = false;

		if (sprSet == nullptr && sprSetLoadFuture.valid() && sprSetLoadFuture._Is_ready())
		{
			sprSet = sprSetLoadFuture.get();

			renderer->Aet().SetSprGetter([&](const Graphics::Aet::VideoSource& source) -> Render::TexSpr
			{
				return Render::SprSetNameStringSprGetter(source, sprSet.get());
			});
		}

		if (aetSet == nullptr && aetSetLoadFuture.valid() && aetSetLoadFuture._Is_ready())
		{
			aetSet = aetSetLoadFuture.get();
			auto tryFindLayer = [&](std::string_view layerName) -> std::shared_ptr<Graphics::Aet::Layer>
			{
				return (aetSet == nullptr || aetSet->GetScenes().empty()) ? nullptr : aetSet->GetScenes().front()->FindLayer(layerName);
			};

			layerCache.FrameUp = tryFindLayer("frame_up_t");
			layerCache.FrameBottom = tryFindLayer("frame_bottom_t");
			layerCache.LifeGauge = tryFindLayer("life_gauge");
			layerCache.SongEnergyBase = tryFindLayer("song_energy_base_t");
			layerCache.SongIconLoop = tryFindLayer("song_icon_loop");
			layerCache.LevelInfoEasy = tryFindLayer("level_info_easy");
			layerCache.LevelInfoNormal = tryFindLayer("level_info_normal");
			layerCache.LevelInfoHard = tryFindLayer("level_info_hard");
			layerCache.LevelInfoExtreme = tryFindLayer("level_info_extreme");
			layerCache.LevelInfoExExtreme = tryFindLayer("level_info_extreme_extra");
			layerCache.SongInfoLoop = tryFindLayer("song_icon_loop");
		}
	}
}

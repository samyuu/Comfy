#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

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
		aetSet = IO::File::Load<Aet::AetSet>(aetSetFilePath);

		if (aetSet == nullptr || aetSet->GetScenes().empty())
			return;

		layerCache.FrameUp = aetSet->GetScenes().front()->FindLayer("frame_up_f");
		layerCache.FrameBottom = aetSet->GetScenes().front()->FindLayer("frame_bottom_f");
		layerCache.LifeGauge = aetSet->GetScenes().front()->FindLayer("life_gauge");
		layerCache.SongEnergyBase = aetSet->GetScenes().front()->FindLayer("song_energy_base_f");
		layerCache.SongIconLoop = aetSet->GetScenes().front()->FindLayer("song_icon_loop");
		layerCache.LevelInfoEasy = aetSet->GetScenes().front()->FindLayer("level_info_easy");
		layerCache.SongInfoLoop = aetSet->GetScenes().front()->FindLayer("song_icon_loop");
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
		tryDrawLayer(layerCache.LevelInfoEasy, 0.0f);
		tryDrawLayer(layerCache.SongIconLoop, 0.0f);
	}

	void TargetRenderWindow::UpdateContentLoading()
	{
		if (sprSet != nullptr)
			loadingContent = false;

		if (sprSet == nullptr && sprSetLoadFuture.valid() && sprSetLoadFuture._Is_ready())
		{
			sprSet = sprSetLoadFuture.get();

			renderer->Aet().SetSprGetter([&](const Graphics::Aet::VideoSource& source) -> Render::TexSpr
			{
				return Render::SprSetNameStringSprGetter(source, sprSet.get());
			});
		}
	}
}

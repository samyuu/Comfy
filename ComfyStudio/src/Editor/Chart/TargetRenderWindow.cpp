#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow()
	{
		spriteGetterFunction = [this](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr) { return false; };

		renderer = std::make_unique<D3D11::Renderer2D>();
		aetRenderer = std::make_unique<Aet::AetRenderer>(renderer.get());
		aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
	}

	TargetRenderWindow::~TargetRenderWindow()
	{
	}

	void TargetRenderWindow::OnInitialize()
	{
		SetKeepAspectRatio(true);
		SetTargetAspectRatio(renderSize.x / renderSize.y);

		sprSetLoader.LoadAsync();

		aetSet = std::make_unique<Aet::AetSet>();
		aetSetLoader.LoadSync();
		aetSetLoader.Read(*aetSet);
		aetSetLoader.FreeData();

		layerCache.FrameUp = aetSet->GetScenes().front()->FindLayer("frame_up_f");
		layerCache.FrameBottom = aetSet->GetScenes().front()->FindLayer("frame_bottom_f");
		layerCache.LifeGauge = aetSet->GetScenes().front()->FindLayer("life_gauge");
		layerCache.SongEnergyBase = aetSet->GetScenes().front()->FindLayer("song_energy_base_f");
		layerCache.SongIconLoop = aetSet->GetScenes().front()->FindLayer("song_icon_loop");
		layerCache.LevelInfoEasy = aetSet->GetScenes().front()->FindLayer("level_info_easy");
		layerCache.SongInfoLoop = aetSet->GetScenes().front()->FindLayer("song_icon_loop");
	}

	void TargetRenderWindow::OnDrawGui()
	{
	}

	void TargetRenderWindow::PostDrawGui()
	{
	}

	void TargetRenderWindow::OnUpdateInput()
	{
	}

	void TargetRenderWindow::OnUpdate()
	{
		if (loadingContent)
			UpdateContentLoading();
	}

	void TargetRenderWindow::OnRender()
	{
		owningRenderTarget->Bind();
		{
			D3D11::D3D.SetViewport(owningRenderTarget->GetSize());
			owningRenderTarget->Clear(GetColorVec4(EditorColor_DarkClear));

			camera.UpdateMatrices();
			renderer->Begin(camera);
			{
				RenderBackground();

				aetRenderer->RenderLayer(layerCache.FrameUp.get(), 0.0f);
				aetRenderer->RenderLayer(layerCache.FrameBottom.get(), 0.0f);
				aetRenderer->RenderLayer(layerCache.LifeGauge.get(), 0.0f);
				aetRenderer->RenderLayer(layerCache.SongEnergyBase.get(), 100.0f);
				aetRenderer->RenderLayer(layerCache.SongIconLoop.get(), 0.0f);
				aetRenderer->RenderLayer(layerCache.LevelInfoEasy.get(), 0.0f);
				aetRenderer->RenderLayer(layerCache.SongIconLoop.get(), 0.0f);
			}
			renderer->End();
		}
		owningRenderTarget->UnBind();
	}

	void TargetRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		camera.ProjectionSize = vec2(size);
		camera.Position = vec2(0.0f, 0.0f);
		camera.Zoom = camera.ProjectionSize.x / renderSize.x;
	}

	void TargetRenderWindow::RenderBackground()
	{
		checkerboardGrid.Size = renderSize;
		checkerboardGrid.Render(*renderer);

		renderer->Draw(
			vec2(0.0f),
			renderSize,
			vec4(0.0f, 0.0f, 0.0f, .25f));
	}

	void TargetRenderWindow::UpdateContentLoading()
	{
		if (sprSet != nullptr)
			loadingContent = false;

		if (sprSet == nullptr && sprSetLoader.GetIsLoaded())
		{
			sprSet = std::make_unique<SprSet>();
			sprSetLoader.Parse(*sprSet);
			sprSet->TexSet->UploadAll(sprSet.get());
			sprSetLoader.FreeData();

			spriteGetterFunction = [this](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr) 
			{ 
				return Aet::AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), source, outTex, outSpr); 
			};
			aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
		}
	}
}

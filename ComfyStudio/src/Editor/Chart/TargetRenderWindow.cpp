#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"

namespace Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow()
	{
		spriteGetterFunction = [this](const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr) { return false; };

		renderer = MakeUnique<D3D_Renderer2D>();
		aetRenderer = MakeUnique<AetRenderer>(renderer.get());
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

		aetSet = MakeUnique<AetSet>();
		aetSetLoader.LoadSync();
		aetSetLoader.Read(aetSet.get());
		aetSetLoader.FreeData();

		layerCache.FrameUp = aetSet->front()->FindLayer("frame_up_f");
		layerCache.FrameBottom = aetSet->front()->FindLayer("frame_bottom_f");
		layerCache.LifeGauge = aetSet->front()->FindLayer("life_gauge");
		layerCache.SongEnergyBase = aetSet->front()->FindLayer("song_energy_base_f");
		layerCache.SongIconLoop = aetSet->front()->FindLayer("song_icon_loop");
		layerCache.LevelInfoEasy = aetSet->front()->FindLayer("level_info_easy");
		layerCache.SongInfoLoop = aetSet->front()->FindLayer("song_icon_loop");
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
		renderTarget->Bind();
		{
			D3D.SetViewport(renderTarget->GetSize());
			renderTarget->Clear(GetColorVec4(EditorColor_DarkClear));

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
		renderTarget->UnBind();
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
		checkerboardGrid.Render(renderer.get());

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
			sprSet = MakeUnique<SprSet>();
			sprSetLoader.Parse(sprSet.get());
			sprSet->TxpSet->UploadAll(sprSet.get());
			sprSetLoader.FreeData();

			spriteGetterFunction = [this](const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), identifier, outTxp, outSpr); };
			aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
		}
	}
}
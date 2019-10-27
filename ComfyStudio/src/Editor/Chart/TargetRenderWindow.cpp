#include "TargetRenderWindow.h"
#include "Editor/Core/Theme.h"

namespace Editor
{
	using namespace Graphics;

	TargetRenderWindow::TargetRenderWindow()
	{
		spriteGetterFunction = [this](const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr) { return false; };

		renderer = MakeUnique<Renderer2D>();
		aetRenderer = MakeUnique<AetRenderer>(renderer.get());
		aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
	}

	TargetRenderWindow::~TargetRenderWindow()
	{
	}

	void TargetRenderWindow::OnInitialize()
	{
		renderer->Initialize();
		SetKeepAspectRatio(true);
		SetTargetAspectRatio(renderSize.x / renderSize.y);

		sprSetLoader.LoadAsync();

		aetSet = MakeUnique<AetSet>();
		aetSetLoader.LoadSync();
		aetSetLoader.Read(aetSet.get());
		aetSetLoader.FreeData();

		aetObjCache.FrameUp = aetSet->front()->FindObj("frame_up_f");
		aetObjCache.FrameBottom = aetSet->front()->FindObj("frame_bottom_f");
		aetObjCache.LifeGauge = aetSet->front()->FindObj("life_gauge");
		aetObjCache.SongEnergyBase = aetSet->front()->FindObj("song_energy_base_f");
		aetObjCache.SongIconLoop = aetSet->front()->FindObj("song_icon_loop");
		aetObjCache.LevelInfoEasy = aetSet->front()->FindObj("level_info_easy");
		aetObjCache.SongInfoLoop = aetSet->front()->FindObj("song_icon_loop");
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
		renderTarget.Bind();
		{
			RenderCommand::SetViewport(renderTarget.GetSize());
			RenderCommand::SetClearColor(GetColorVec4(EditorColor_DarkClear));
			RenderCommand::Clear(ClearTarget_ColorBuffer);

			camera.UpdateMatrices();
			renderer->Begin(camera);
			{
				RenderBackground();

				aetRenderer->RenderAetObj(aetObjCache.FrameUp.get(), 0.0f);
				aetRenderer->RenderAetObj(aetObjCache.FrameBottom.get(), 0.0f);
				aetRenderer->RenderAetObj(aetObjCache.LifeGauge.get(), 0.0f);
				aetRenderer->RenderAetObj(aetObjCache.SongEnergyBase.get(), 100.0f);
				aetRenderer->RenderAetObj(aetObjCache.SongIconLoop.get(), 0.0f);
				aetRenderer->RenderAetObj(aetObjCache.LevelInfoEasy.get(), 0.0f);
				aetRenderer->RenderAetObj(aetObjCache.SongIconLoop.get(), 0.0f);
			}
			renderer->End();
		}
		renderTarget.UnBind();
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
			sprSet->TxpSet->UploadAll();
			sprSetLoader.FreeData();

			spriteGetterFunction = [this](const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), identifier, outTxp, outSpr); };
			aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
		}
	}
}
#include "TargetRenderWindow.h"
#include "Editor/Theme.h"

namespace Editor
{
	using namespace FileSystem;
	using namespace Auth2D;

	TargetRenderWindow::TargetRenderWindow()
	{
		spriteGetterFunction = [this](AetSprite* inSprite, Texture** outTexture, Sprite** outSprite) { return false; };

		renderer = std::make_unique<Renderer2D>();
		aetRenderer = std::make_unique<AetRenderer>(renderer.get());
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

		aetSet = std::make_unique<AetSet>();
		aetSetLoader.LoadSync();
		aetSetLoader.Read(aetSet.get());
		aetSetLoader.FreeData();
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

				aetRenderer->RenderAetObj(aetSet->front().GetObj("frame_up_f"), 0.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("frame_bottom_f"), 0.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("life_gauge"), 0.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("song_energy_base_f"), 100.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("song_icon_loop"), 0.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("level_info_easy"), 0.0f);
				aetRenderer->RenderAetObj(aetSet->front().GetObj("song_icon_loop"), 0.0f);
			}
			renderer->End();
		}
		renderTarget.UnBind();
	}

	void TargetRenderWindow::OnResize(int width, int height)
	{
		RenderWindowBase::OnResize(width, height);

		camera.ProjectionSize = vec2(width, height);
		camera.Position = vec2(0.0f, 0.0f);
		camera.Zoom = width / renderSize.x;
	}

	void TargetRenderWindow::RenderBackground()
	{
		checkerboardGrid.Size = renderSize;
		checkerboardGrid.Render(renderer.get());
	}

	void TargetRenderWindow::UpdateContentLoading()
	{
		if (sprSet != nullptr)
			loadingContent = false;

		if (sprSet == nullptr && sprSetLoader.GetIsLoaded())
		{
			sprSet = std::make_unique<SprSet>();
			sprSetLoader.Parse(sprSet.get());
			sprSet->TxpSet->UploadAll();
			sprSetLoader.FreeData();

			spriteGetterFunction = [this](AetSprite* inSprite, Texture** outTexture, Sprite** outSprite) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), inSprite, outTexture, outSprite); };
			aetRenderer->SetSpriteGetterFunction(&spriteGetterFunction);
		}
	}
}
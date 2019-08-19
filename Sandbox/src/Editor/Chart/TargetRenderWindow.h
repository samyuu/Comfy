#pragma once
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "FileSystem/FileLoader.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"

namespace Editor
{
	using namespace FileSystem;

	class TargetRenderWindow : public RenderWindowBase
	{
	public:
		TargetRenderWindow();
		~TargetRenderWindow();

	protected:
		void OnInitialize() override;
		void OnDrawGui() override;
		void PostDrawGui() override;
		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(int width, int height) override;

	protected:
		CheckerboardGrid checkerboardGrid;

		const vec2 renderSize = vec2(1920.0f, 1080.0f);
		Graphics::OrthographicCamera camera;

		Graphics::Auth2D::SpriteGetterFunction spriteGetterFunction;
		UniquePtr<Graphics::Auth2D::Renderer2D> renderer;
		UniquePtr<Graphics::Auth2D::AetRenderer> aetRenderer;

		// TODO: ps4_gam
		FileLoader aetSetLoader = { "dev_rom/2d/aet_gam_cmn.bin" };
		FileLoader sprSetLoader = { "dev_rom/2d/spr_gam_cmn.bin" };

		UniquePtr<AetSet> aetSet;
		UniquePtr<SprSet> sprSet;

		void RenderBackground();

	private:
		bool loadingContent = true;

		void UpdateContentLoading();
	};
}
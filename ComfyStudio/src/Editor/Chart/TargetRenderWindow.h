#pragma once
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "FileSystem/FileLoader.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "Graphics/Auth2D/AetSet.h"
#include "Graphics/SprSet.h"

namespace Editor
{
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
		void OnResize(ivec2 size) override;

	protected:
		CheckerboardGrid checkerboardGrid;

		const vec2 renderSize = vec2(1920.0f, 1080.0f);
		Graphics::OrthographicCamera camera;

		Graphics::SpriteGetterFunction spriteGetterFunction;
		UniquePtr<Graphics::Renderer2D> renderer;
		UniquePtr<Graphics::AetRenderer> aetRenderer;

		// TODO: ps4_gam
		FileSystem::FileLoader aetSetLoader = { "dev_rom/2d/aet_gam_cmn.bin" };
		FileSystem::FileLoader sprSetLoader = { "dev_rom/2d/spr_gam_cmn.bin" };

		UniquePtr<Graphics::AetSet> aetSet;
		UniquePtr<Graphics::SprSet> sprSet;

		struct /* AetObjCache */
		{
			RefPtr<Graphics::AetObj> FrameUp, FrameBottom;
			RefPtr<Graphics::AetObj> LifeGauge;
			RefPtr<Graphics::AetObj> SongEnergyBase;
			RefPtr<Graphics::AetObj> SongIconLoop;
			RefPtr<Graphics::AetObj> LevelInfoEasy;
			RefPtr<Graphics::AetObj> SongInfoLoop;
		} aetObjCache;

		void RenderBackground();

	private:
		bool loadingContent = true;

		void UpdateContentLoading();
	};
}
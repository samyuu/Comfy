#pragma once
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "FileSystem/FileLoader.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/AetRenderer.h"
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
		UniquePtr<Graphics::D3D_Renderer2D> renderer;
		UniquePtr<Graphics::AetRenderer> aetRenderer;

		// TODO: ps4_gam
		FileSystem::FileLoader aetSetLoader = { "dev_rom/2d/aet_gam_cmn.bin" };
		FileSystem::FileLoader sprSetLoader = { "dev_rom/2d/spr_gam_cmn.bin" };

		UniquePtr<Graphics::AetSet> aetSet;
		UniquePtr<Graphics::SprSet> sprSet;

		struct /* InternalLayerCache */
		{
			RefPtr<Graphics::AetLayer> FrameUp, FrameBottom;
			RefPtr<Graphics::AetLayer> LifeGauge;
			RefPtr<Graphics::AetLayer> SongEnergyBase;
			RefPtr<Graphics::AetLayer> SongIconLoop;
			RefPtr<Graphics::AetLayer> LevelInfoEasy;
			RefPtr<Graphics::AetLayer> SongInfoLoop;
		} layerCache;

		void RenderBackground();

	private:
		bool loadingContent = true;

		void UpdateContentLoading();
	};
}
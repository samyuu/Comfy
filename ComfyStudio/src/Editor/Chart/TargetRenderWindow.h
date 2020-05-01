#pragma once
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "IO/AsyncFileLoader.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Aet/AetRenderer.h"

namespace Comfy::Editor
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

		Graphics::Aet::SpriteGetterFunction spriteGetterFunction;
		UniquePtr<Graphics::GPU_Renderer2D> renderer;
		UniquePtr<Graphics::Aet::AetRenderer> aetRenderer;

		// TODO: ps4_gam
		IO::AsyncFileLoader aetSetLoader = { "dev_rom/2d/aet_gam_cmn.bin" };
		IO::AsyncFileLoader sprSetLoader = { "dev_rom/2d/spr_gam_cmn.bin" };

		UniquePtr<Graphics::Aet::AetSet> aetSet;
		UniquePtr<Graphics::SprSet> sprSet;

		struct /* InternalLayerCache */
		{
			RefPtr<Graphics::Aet::Layer> FrameUp, FrameBottom;
			RefPtr<Graphics::Aet::Layer> LifeGauge;
			RefPtr<Graphics::Aet::Layer> SongEnergyBase;
			RefPtr<Graphics::Aet::Layer> SongIconLoop;
			RefPtr<Graphics::Aet::Layer> LevelInfoEasy;
			RefPtr<Graphics::Aet::Layer> SongInfoLoop;
		} layerCache;

		void RenderBackground();

	private:
		bool loadingContent = true;

		void UpdateContentLoading();
	};
}

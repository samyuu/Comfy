#pragma once
#include "Window/RenderWindow.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "IO/File.h"
#include "Render/Render.h"

namespace Comfy::Studio::Editor
{
	class TargetRenderWindow : public RenderWindow
	{
	public:
		TargetRenderWindow() = default;
		~TargetRenderWindow() = default;

	public:
		ImTextureID GetTextureID() const override;

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override;
		void OnFirstFrame() override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;
		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	protected:
		CheckerboardGrid backgroundCheckerboard;
		float backgroundDim = 0.25f;

		const vec2 renderSize = vec2(1920.0f, 1080.0f);
		Render::Camera2D camera;

		std::unique_ptr<Render::Renderer2D> renderer;
		std::unique_ptr<Render::RenderTarget2D> renderTarget;

		// TODO: ps4_gam
		std::string_view aetSetFilePath = "dev_rom/2d/aet_gam_cmn.bin";
		std::string_view sprSetFilePath = "dev_rom/2d/spr_gam_cmn.bin";
		std::future<std::unique_ptr<Graphics::SprSet>> sprSetLoadFuture;

		std::unique_ptr<Graphics::Aet::AetSet> aetSet;
		std::unique_ptr<Graphics::SprSet> sprSet;

		struct LayerCacheData
		{
			std::shared_ptr<Graphics::Aet::Layer> FrameUp, FrameBottom;
			std::shared_ptr<Graphics::Aet::Layer> LifeGauge;
			std::shared_ptr<Graphics::Aet::Layer> SongEnergyBase;
			std::shared_ptr<Graphics::Aet::Layer> SongIconLoop;
			std::shared_ptr<Graphics::Aet::Layer> LevelInfoEasy;
			std::shared_ptr<Graphics::Aet::Layer> LevelInfoNormal;
			std::shared_ptr<Graphics::Aet::Layer> LevelInfoHard;
			std::shared_ptr<Graphics::Aet::Layer> LevelInfoExtreme;
			std::shared_ptr<Graphics::Aet::Layer> LevelInfoExExtreme;
			std::shared_ptr<Graphics::Aet::Layer> SongInfoLoop;
		} layerCache;

		void RenderBackground();
		void RenderTestAet();

	private:
		bool loadingContent = true;

		void UpdateContentLoading();
	};
}

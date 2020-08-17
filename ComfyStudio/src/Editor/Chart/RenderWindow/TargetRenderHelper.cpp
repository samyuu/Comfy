#include "TargetRenderHelper.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth2D/Font/FontMap.h"
#include "IO/File.h"

namespace Comfy
{
	// TODO: Make generic ComfyLib algorithm (?)
	template <typename Future, typename Type>
	bool GetFutureIfReady(Future& inFuture, Type& outValue)
	{
		if (inFuture.valid() && inFuture._Is_ready())
		{
			outValue = inFuture.get();
			return true;
		}
		else
		{
			return false;
		}
	}
}

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	struct TargetRenderHelper::Impl
	{
		std::future<std::unique_ptr<AetSet>> AetGameCommonFuture = IO::File::LoadAsync<AetSet>("dev_rom/2d/aet_gam_cmn.bin");
		std::future<std::unique_ptr<SprSet>> SprGameCommonFuture = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_gam_cmn.farc<spr_gam_cmn.bin>");
		std::future<std::unique_ptr<AetSet>> AetGameFuture = IO::File::LoadAsync<AetSet>("dev_rom/2d/aet_ps4_game.bin");
		std::future<std::unique_ptr<SprSet>> SprGameFuture = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_ps4_game.farc<spr_ps4_game.bin>");
		std::future<std::unique_ptr<SprSet>> SprFont36Future = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_fnt_36.farc<spr_fnt_36.bin>");
		std::future<std::unique_ptr<FontMap>> FontMapFuture = IO::File::LoadAsync<Graphics::FontMap>("dev_rom/fontmap.farc<fontmap.bin>");

		std::unique_ptr<AetSet> AetGameCommon = nullptr;
		std::unique_ptr<SprSet> SprGameCommon = nullptr;
		std::unique_ptr<AetSet> AetGame = nullptr;
		std::unique_ptr<SprSet> SprGame = nullptr;
		std::unique_ptr<SprSet> SprFont36 = nullptr;
		std::unique_ptr<FontMap> FontMap = nullptr;
		size_t Font36Index = std::numeric_limits<size_t>::max();

		BitmapFont* Font36() const
		{
			return (SprFont36 == nullptr || FontMap == nullptr) ? nullptr : IndexOrNull(Font36Index, FontMap->Fonts);
		}

		struct LayerCache
		{
			std::shared_ptr<Aet::Layer>
				FrameUp,
				FrameBottom,
				LifeGauge,
				SongEnergyBase,
				SongEnergyNormal,
				SongIconLoop,
				LevelInfoEasy,
				LevelInfoNormal,
				LevelInfoHard,
				LevelInfoExtreme,
				LevelInfoExExtreme,
				SongTitle;

			std::shared_ptr<Aet::Layer>
				PracticeGaugeBase,
				PracticeGaugeTime,
				PracticeGaugeBorderPlay,
				PracticeGaugeBorderRestart;

		} Layers = {};

	public:
		void UpdateAsyncLoading()
		{
			auto findLayer = [](Graphics::AetSet& aetSet, std::string_view layerName, size_t sceneIndex = 0)
			{
				return InBounds(sceneIndex, aetSet.GetScenes()) ? aetSet.GetScenes()[sceneIndex]->FindLayer(layerName) : nullptr;
			};

			if (GetFutureIfReady(AetGameCommonFuture, AetGameCommon) && AetGameCommon != nullptr)
			{
				Layers.FrameUp = findLayer(*AetGameCommon, "frame_up_t");
				Layers.FrameBottom = findLayer(*AetGameCommon, "frame_bottom_t");
				Layers.LifeGauge = findLayer(*AetGameCommon, "life_gauge");
				Layers.SongEnergyBase = findLayer(*AetGameCommon, "song_energy_base_t");
				Layers.SongEnergyNormal = findLayer(*AetGameCommon, "song_energy_normal");
				Layers.SongIconLoop = findLayer(*AetGameCommon, "song_icon_loop");
				Layers.LevelInfoEasy = findLayer(*AetGameCommon, "level_info_easy");
				Layers.LevelInfoNormal = findLayer(*AetGameCommon, "level_info_normal");
				Layers.LevelInfoHard = findLayer(*AetGameCommon, "level_info_hard");
				Layers.LevelInfoExtreme = findLayer(*AetGameCommon, "level_info_extreme");
				Layers.LevelInfoExExtreme = findLayer(*AetGameCommon, "level_info_extreme_extra");
				Layers.SongTitle = findLayer(*AetGameCommon, "p_song_title_lt");
			}
			GetFutureIfReady(SprGameCommonFuture, SprGameCommon);
			if (GetFutureIfReady(AetGameFuture, AetGame))
			{
				Layers.PracticeGaugeBase = findLayer(*AetGame, "prc_gauge_base");
				Layers.PracticeGaugeTime = findLayer(*AetGame, "prc_gauge_time");
				Layers.PracticeGaugeBorderPlay = findLayer(*AetGame, "prc_gauge_border_play");
				Layers.PracticeGaugeBorderRestart = findLayer(*AetGame, "prc_gauge_border_restart");
			}
			GetFutureIfReady(SprGameFuture, SprGame);
			GetFutureIfReady(SprFont36Future, SprFont36);
			if (GetFutureIfReady(FontMapFuture, FontMap) && FontMap != nullptr)
				Font36Index = FindIndexOf(FontMap->Fonts, [](auto& font) { return font.GetFontSize() == ivec2(36); });

			if (const auto font36 = Font36(); font36 != nullptr)
			{
				if (font36->Texture == nullptr && SprFont36 != nullptr && !SprFont36->TexSet.Textures.empty())
					font36->Texture = SprFont36->TexSet.Textures.front();
			}
		}

		void DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const
		{
			// TODO: Find a better solution for this, maybe store tex shared_ptr + spr as mutable VideoSource member (?)
			renderer.Aet().SetSprGetter([this](const Aet::VideoSource& source) -> Render::TexSpr
			{
				if (auto result = Render::SprSetNameStringSprGetter(source, SprGameCommon.get()); result.Tex != nullptr && result.Spr != nullptr)
					return result;

				if (auto result = Render::SprSetNameStringSprGetter(source, SprGame.get()); result.Tex != nullptr && result.Spr != nullptr)
					return result;

				return Render::NullSprGetter(source);
			});

			auto tryDrawLayer = [&](const auto& layer, frame_t frame)
			{
				if (layer != nullptr)
					renderer.Aet().DrawLayer(*layer, frame);
			};

			if (SprGameCommon != nullptr)
			{
				tryDrawLayer(Layers.FrameUp, 0.0f);
				tryDrawLayer(Layers.FrameBottom, 0.0f);
				tryDrawLayer(Layers.LifeGauge, 0.0f);
				tryDrawLayer(Layers.SongIconLoop, 0.0f);
				tryDrawLayer(Layers.LevelInfoHard, 0.0f);
			}

			if (SprGame != nullptr)
			{
				const auto progress = std::clamp(static_cast<f32>(hud.PlaybackTime / hud.Duration), 0.0f, 1.0f);
				const auto progressOnStart = std::clamp(static_cast<f32>(hud.PlaybackTimeOnStart / hud.Duration), 0.0f, 1.0f);

				tryDrawLayer(Layers.PracticeGaugeBase, 0.0f);
				tryDrawLayer(Layers.PracticeGaugeTime, progress * 100.0f);
				tryDrawLayer(Layers.PracticeGaugeBorderRestart, (hud.IsPlayback ? progressOnStart : progress) * 100.0f);
			}

			if (const auto* font = Font36(); font != nullptr && !hud.SongName.empty())
			{
				const auto songNameTransform = (Layers.SongTitle != nullptr && Layers.SongTitle->LayerVideo != nullptr) ?
					Aet::Util::GetTransformAt(*Layers.SongTitle->LayerVideo, 0.0f) : Transform2D(vec2(0.0f));

				renderer.Font().DrawBorder(*font, hud.SongName, songNameTransform);
			}
		}
	};

	TargetRenderHelper::TargetRenderHelper() : impl(std::make_unique<Impl>())
	{
	}

	TargetRenderHelper::~TargetRenderHelper() = default;

	void TargetRenderHelper::UpdateAsyncLoading()
	{
		impl->UpdateAsyncLoading();
	}

	void TargetRenderHelper::DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const
	{
		impl->DrawHUD(renderer, hud);
	}
}

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
		size_t FontPracticeNumIndex = std::numeric_limits<size_t>::max();

		BitmapFont* Font36() const
		{
			return (SprFont36 == nullptr || FontMap == nullptr) ? nullptr : IndexOrNull(Font36Index, FontMap->Fonts);
		}

		BitmapFont* FontPracticeNum() const
		{
			return (SprGame == nullptr || FontMap == nullptr) ? nullptr : IndexOrNull(FontPracticeNumIndex, FontMap->Fonts);
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
				PracticeGaugeBaseNumPointMin,
				PracticeGaugeBaseNumPointSec,
				PracticeGaugeBaseNumPointMS,
				PracticeGaugeTime,
				PracticeGaugeBorderPlay,
				PracticeGaugeBorderRestart;

		} Layers = {};

		struct SpriteCache
		{
			Spr* PracticeNumbers;
		} Sprites = {};

	public:
		void UpdateAsyncLoading()
		{
			auto findLayer = [](AetSet& aetSet, std::string_view layerName, size_t sceneIndex = 0)
			{
				return InBounds(sceneIndex, aetSet.GetScenes()) ? aetSet.GetScenes()[sceneIndex]->FindLayer(layerName) : nullptr;
			};

			auto findSprite = [](SprSet& sprSet, std::string_view spriteName)
			{
				return FindIfOrNull(sprSet.Sprites, [&](const auto& spr) {return spr.Name == spriteName; });
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

			if (GetFutureIfReady(SprGameCommonFuture, SprGameCommon) && SprGameCommon != nullptr)
			{

			}

			if (GetFutureIfReady(AetGameFuture, AetGame) && AetGame != nullptr)
			{
				Layers.PracticeGaugeBase = findLayer(*AetGame, "prc_gauge_base");
				Layers.PracticeGaugeTime = findLayer(*AetGame, "prc_gauge_time");
				Layers.PracticeGaugeBorderPlay = findLayer(*AetGame, "prc_gauge_border_play");
				Layers.PracticeGaugeBorderRestart = findLayer(*AetGame, "prc_gauge_border_restart");

				if (Layers.PracticeGaugeBase != nullptr && Layers.PracticeGaugeBase->GetCompItem() != nullptr)
				{
					Layers.PracticeGaugeBaseNumPointMin = Layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_min_rt");
					Layers.PracticeGaugeBaseNumPointSec = Layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_sec_rt");
					Layers.PracticeGaugeBaseNumPointMS = Layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_frm_rt");
				}
			}

			if (GetFutureIfReady(SprGameFuture, SprGame) && SprGame != nullptr)
			{
				Sprites.PracticeNumbers = findSprite(*SprGame, "PRC_NUM24X36");
			}

			GetFutureIfReady(SprFont36Future, SprFont36);

			if (GetFutureIfReady(FontMapFuture, FontMap) && FontMap != nullptr)
			{
				Font36Index = FindIndexOf(FontMap->Fonts, [](auto& font) { return font.GetFontSize() == ivec2(36); });
				FontPracticeNumIndex = FindIndexOf(FontMap->Fonts, [](auto& font) { return font.GetFontSize() == ivec2(24, 30); });
			}

			if (const auto font36 = Font36(); font36 != nullptr)
			{
				if (font36->Texture == nullptr && !SprFont36->TexSet.Textures.empty())
					font36->Texture = SprFont36->TexSet.Textures.front();
			}

			if (const auto fontNum = FontPracticeNum(); fontNum != nullptr && fontNum->Texture == nullptr)
			{
				if (Sprites.PracticeNumbers != nullptr && InBounds(Sprites.PracticeNumbers->TextureIndex, SprGame->TexSet.Textures))
				{
					fontNum->Texture = SprGame->TexSet.Textures[Sprites.PracticeNumbers->TextureIndex];
					fontNum->SpritePixelRegion = Sprites.PracticeNumbers->PixelRegion;
				}
			}
		}

		void DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const
		{
			// TODO: Find a better solution for this, maybe store tex shared_ptr + spr as mutable VideoSource member (?)
			renderer.Aet().SetSprGetter([this](const Aet::VideoSource& source) -> Render::TexSprView
			{
				if (auto result = Render::SprSetNameStringSprGetter(source, SprGameCommon.get()); result)
					return result;

				if (auto result = Render::SprSetNameStringSprGetter(source, SprGame.get()); result)
					return result;

				return Render::NullSprGetter(source);
			});

			auto tryDrawLayer = [&](const std::shared_ptr<Aet::Layer>& layer, frame_t frame) -> void
			{
				if (layer != nullptr) { renderer.Aet().DrawLayer(*layer, frame); }
			};

			auto tryDrawSpr = [&](const SprSet* sprSet, const Spr* spr, const Transform2D& transform) -> void
			{
				if (sprSet != nullptr && spr != nullptr && InBounds(spr->TextureIndex, sprSet->TexSet.Textures))
					renderer.Aet().DrawSpr(*sprSet->TexSet.Textures[spr->TextureIndex], *spr, transform);
			};

			auto tryGetTransform = [&](const std::shared_ptr<Aet::Layer>& layer, frame_t frame) -> Transform2D
			{
				return (layer != nullptr && layer->LayerVideo != nullptr) ? Aet::Util::GetTransformAt(*layer->LayerVideo, frame) : Transform2D(vec2(0.0f));
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
				tryDrawLayer(Layers.PracticeGaugeBase, 0.0f);

				if (Layers.PracticeGaugeBase != nullptr && Sprites.PracticeNumbers != nullptr)
				{
					if (const auto* font = FontPracticeNum(); font != nullptr)
					{
						constexpr auto minTime = TimeSpan::Zero();
						constexpr auto maxTime = TimeSpan::FromMinutes(9.0) + TimeSpan::FromSeconds(59.0) + TimeSpan::FromMilliseconds(999.0);
						const auto timeFormat = std::clamp(hud.PlaybackTime, minTime, maxTime).FormatTime();

						// HACK: Indexing into an external string...
						char minutes[2] = { timeFormat[1] };
						char seconds[3] = { timeFormat[3], timeFormat[4] };
						char milliseconds[3] = { timeFormat[6], timeFormat[7] };

						renderer.Font().Draw(*font, minutes, tryGetTransform(Layers.PracticeGaugeBaseNumPointMin, 0.0f));
						renderer.Font().Draw(*font, seconds, tryGetTransform(Layers.PracticeGaugeBaseNumPointSec, 0.0f));
						renderer.Font().Draw(*font, milliseconds, tryGetTransform(Layers.PracticeGaugeBaseNumPointMS, 0.0f));
					}
				}

				const auto progress = std::clamp(static_cast<f32>(hud.PlaybackTime / hud.Duration), 0.0f, 1.0f);
				const auto progressOnStart = std::clamp(static_cast<f32>(hud.PlaybackTimeOnStart / hud.Duration), 0.0f, 1.0f);

				tryDrawLayer(Layers.PracticeGaugeTime, progress * 100.0f);
				tryDrawLayer(Layers.PracticeGaugeBorderRestart, (hud.IsPlayback ? progressOnStart : progress) * 100.0f);
			}

			if (const auto* font = Font36(); font != nullptr && !hud.SongName.empty())
				renderer.Font().DrawBorder(*font, hud.SongName, tryGetTransform(Layers.SongTitle, 0.0f));
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

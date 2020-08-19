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

			if (GetFutureIfReady(aetGameCommonFuture, aetGameCommon) && aetGameCommon != nullptr)
			{
				layers.FrameUp = findLayer(*aetGameCommon, "frame_up_t");
				layers.FrameBottom = findLayer(*aetGameCommon, "frame_bottom_t");
				layers.LifeGauge = findLayer(*aetGameCommon, "life_gauge");
				layers.SongEnergyBase = findLayer(*aetGameCommon, "song_energy_base_t");
				layers.SongEnergyNormal = findLayer(*aetGameCommon, "song_energy_normal");
				layers.SongIconLoop = findLayer(*aetGameCommon, "song_icon_loop");
				layers.LevelInfoEasy = findLayer(*aetGameCommon, "level_info_easy");
				layers.LevelInfoNormal = findLayer(*aetGameCommon, "level_info_normal");
				layers.LevelInfoHard = findLayer(*aetGameCommon, "level_info_hard");
				layers.LevelInfoExtreme = findLayer(*aetGameCommon, "level_info_extreme");
				layers.LevelInfoExExtreme = findLayer(*aetGameCommon, "level_info_extreme_extra");
				layers.SongTitle = findLayer(*aetGameCommon, "p_song_title_lt");
			}

			if (GetFutureIfReady(sprGameCommonFuture, sprGameCommon) && sprGameCommon != nullptr)
			{

			}

			if (GetFutureIfReady(aetGameFuture, aetGame) && aetGame != nullptr)
			{
				layers.PracticeGaugeBase = findLayer(*aetGame, "prc_gauge_base");
				layers.PracticeGaugeTime = findLayer(*aetGame, "prc_gauge_time");
				layers.PracticeGaugeBorderPlay = findLayer(*aetGame, "prc_gauge_border_play");
				layers.PracticeGaugeBorderRestart = findLayer(*aetGame, "prc_gauge_border_restart");

				if (layers.PracticeGaugeBase != nullptr && layers.PracticeGaugeBase->GetCompItem() != nullptr)
				{
					layers.PracticeGaugeBaseNumPointMin = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_min_rt");
					layers.PracticeGaugeBaseNumPointSec = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_sec_rt");
					layers.PracticeGaugeBaseNumPointMS = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_frm_rt");
				}
			}

			if (GetFutureIfReady(sprGameFuture, sprGame) && sprGame != nullptr)
			{
				sprites.PracticeNumbers = findSprite(*sprGame, "PRC_NUM24X36");
			}

			GetFutureIfReady(sprFont36Future, sprFont36);

			if (GetFutureIfReady(fontMapFuture, fontMap) && fontMap != nullptr)
			{
				font36Index = FindIndexOf(fontMap->Fonts, [](auto& font) { return font.GetFontSize() == ivec2(36); });
				fontPracticeNumIndex = FindIndexOf(fontMap->Fonts, [](auto& font) { return font.GetFontSize() == ivec2(24, 30); });
			}

			if (const auto font36 = GetFont36(); font36 != nullptr)
			{
				if (font36->Texture == nullptr && !sprFont36->TexSet.Textures.empty())
					font36->Texture = sprFont36->TexSet.Textures.front();
			}

			if (const auto fontNum = GetFontPracticeNum(); fontNum != nullptr && fontNum->Texture == nullptr)
			{
				if (sprites.PracticeNumbers != nullptr && InBounds(sprites.PracticeNumbers->TextureIndex, sprGame->TexSet.Textures))
				{
					fontNum->Texture = sprGame->TexSet.Textures[sprites.PracticeNumbers->TextureIndex];
					fontNum->SpritePixelRegion = sprites.PracticeNumbers->PixelRegion;
				}
			}
		}

		void DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const
		{
			if (aetGameCommon == nullptr || sprGameCommon == nullptr || aetGame == nullptr || sprGame == nullptr || sprFont36 == nullptr || fontMap == nullptr)
				return;

			// TODO: Find a better solution for this, maybe store tex shared_ptr + spr as mutable VideoSource member (?)
			renderer.Aet().SetSprGetter([this](const Aet::VideoSource& source) -> Render::TexSprView
			{
				if (auto result = Render::SprSetNameStringSprGetter(source, sprGameCommon.get()); result)
					return result;

				if (auto result = Render::SprSetNameStringSprGetter(source, sprGame.get()); result)
					return result;

				return Render::NullSprGetter(source);
			});

			TryDrawLayer(renderer, layers.FrameUp, 0.0f);
			TryDrawLayer(renderer, layers.FrameBottom, 0.0f);
			TryDrawLayer(renderer, layers.LifeGauge, 0.0f);
			TryDrawLayer(renderer, layers.SongIconLoop, 0.0f);
			TryDrawLayer(renderer, layers.LevelInfoHard, 0.0f);

			TryDrawLayer(renderer, layers.PracticeGaugeBase, 0.0f);
			DrawHUDPracitceTime(renderer, hud.PlaybackTime);

			const auto progress = std::clamp(static_cast<f32>(hud.PlaybackTime / hud.Duration), 0.0f, 1.0f);
			const auto progressOnStart = std::clamp(static_cast<f32>(hud.PlaybackTimeOnStart / hud.Duration), 0.0f, 1.0f);

			TryDrawLayer(renderer, layers.PracticeGaugeTime, progress * 100.0f);
			TryDrawLayer(renderer, layers.PracticeGaugeBorderRestart, (hud.IsPlayback ? progressOnStart : progress) * 100.0f);

			if (const auto* font = GetFont36(); font != nullptr && !hud.SongName.empty())
				renderer.Font().DrawBorder(*font, hud.SongName, TryGetTransform(layers.SongTitle, 0.0f));
		}

	private:
		void DrawHUDPracitceTime(Render::Renderer2D& renderer, TimeSpan playbackTime) const
		{
			if (layers.PracticeGaugeBase == nullptr || sprites.PracticeNumbers == nullptr)
				return;

			const auto* font = GetFontPracticeNum();
			if (font == nullptr)
				return;

			constexpr auto minValidTime = TimeSpan::Zero();
			constexpr auto maxValidTime = TimeSpan::FromMinutes(9.0) + TimeSpan::FromSeconds(59.0) + TimeSpan::FromMilliseconds(999.0);

			const auto totalSeconds = playbackTime.TotalSeconds();
			const auto adjustedTime = (glm::isnan(totalSeconds) || glm::isinf(totalSeconds)) ? minValidTime : playbackTime;
			const auto clampedSeconds = std::clamp(adjustedTime, minValidTime, maxValidTime).TotalSeconds();

			const auto fractionMin = glm::floor(glm::mod(clampedSeconds, 3600.0) / 60.0);
			const auto fractionSec = glm::mod(clampedSeconds, 60.0);
			const auto fractionMS = (fractionSec - glm::floor(fractionSec)) * 100.0;

			char formatBuffer[16];
			sprintf_s(formatBuffer, "%01d:%02d.%02d", static_cast<int>(fractionMin), static_cast<int>(fractionSec), static_cast<int>(fractionMS));

			const auto formattedTime = std::string_view(formatBuffer, std::size(formatBuffer));
			renderer.Font().Draw(*font, formattedTime.substr(0, 1), TryGetTransform(layers.PracticeGaugeBaseNumPointMin, 0.0f));
			renderer.Font().Draw(*font, formattedTime.substr(2, 2), TryGetTransform(layers.PracticeGaugeBaseNumPointSec, 0.0f));
			renderer.Font().Draw(*font, formattedTime.substr(5, 2), TryGetTransform(layers.PracticeGaugeBaseNumPointMS, 0.0f));
		}

		void TryDrawLayer(Render::Renderer2D& renderer, const std::shared_ptr<Aet::Layer>& layer, frame_t frame) const
		{
			if (layer != nullptr) { renderer.Aet().DrawLayer(*layer, frame); }
		}

		void TryDrawSprite(Render::Renderer2D& renderer, const SprSet* sprSet, const Spr* spr, const Transform2D& transform) const
		{
			if (sprSet != nullptr && spr != nullptr && InBounds(spr->TextureIndex, sprSet->TexSet.Textures))
				renderer.Aet().DrawSpr(*sprSet->TexSet.Textures[spr->TextureIndex], *spr, transform);
		}

		Transform2D TryGetTransform(const std::shared_ptr<Aet::Layer>& layer, frame_t frame) const
		{
			return (layer != nullptr && layer->LayerVideo != nullptr) ? Aet::Util::GetTransformAt(*layer->LayerVideo, frame) : Transform2D(vec2(0.0f));
		}

		BitmapFont* GetFont36() const
		{
			return (sprFont36 == nullptr || fontMap == nullptr) ? nullptr : IndexOrNull(font36Index, fontMap->Fonts);
		}

		BitmapFont* GetFontPracticeNum() const
		{
			return (sprGame == nullptr || fontMap == nullptr) ? nullptr : IndexOrNull(fontPracticeNumIndex, fontMap->Fonts);
		}

	private:
		std::future<std::unique_ptr<AetSet>> aetGameCommonFuture = IO::File::LoadAsync<AetSet>("dev_rom/2d/aet_gam_cmn.bin");
		std::future<std::unique_ptr<SprSet>> sprGameCommonFuture = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_gam_cmn.farc<spr_gam_cmn.bin>");
		std::future<std::unique_ptr<AetSet>> aetGameFuture = IO::File::LoadAsync<AetSet>("dev_rom/2d/aet_ps4_game.bin");
		std::future<std::unique_ptr<SprSet>> sprGameFuture = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_ps4_game.farc<spr_ps4_game.bin>");
		std::future<std::unique_ptr<SprSet>> sprFont36Future = IO::File::LoadAsync<SprSet>("dev_rom/2d/spr_fnt_36.farc<spr_fnt_36.bin>");
		std::future<std::unique_ptr<FontMap>> fontMapFuture = IO::File::LoadAsync<FontMap>("dev_rom/fontmap.farc<fontmap.bin>");

		std::unique_ptr<AetSet> aetGameCommon = nullptr;
		std::unique_ptr<SprSet> sprGameCommon = nullptr;
		std::unique_ptr<AetSet> aetGame = nullptr;
		std::unique_ptr<SprSet> sprGame = nullptr;
		std::unique_ptr<SprSet> sprFont36 = nullptr;
		std::unique_ptr<FontMap> fontMap = nullptr;
		size_t font36Index = std::numeric_limits<size_t>::max();
		size_t fontPracticeNumIndex = std::numeric_limits<size_t>::max();

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
		} layers = {};

		struct SpriteCache
		{
			Spr* PracticeNumbers;
		} sprites = {};
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

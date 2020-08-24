#include "TargetRenderHelper.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/Auth2D/Font/FontMap.h"
#include "IO/File.h"
#include <stack>

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

				auto registerTypeLayer = [&](ButtonType button, std::string_view layerName, auto& outArray)
				{
					outArray[static_cast<size_t>(button)] = findLayer(*aetGameCommon, layerName);
				};

				registerTypeLayer(ButtonType::Triangle, "target_sankaku", layers.Targets);
				registerTypeLayer(ButtonType::Square, "target_shikaku", layers.Targets);
				registerTypeLayer(ButtonType::Cross, "target_batsu", layers.Targets);
				registerTypeLayer(ButtonType::Circle, "target_maru", layers.Targets);
				registerTypeLayer(ButtonType::SlideL, "target_slide18_l", layers.Targets);
				registerTypeLayer(ButtonType::SlideR, "target_slide18_r", layers.Targets);

				registerTypeLayer(ButtonType::SlideL, "target_slide26_l", layers.TargetsFrag);
				registerTypeLayer(ButtonType::SlideR, "target_slide26_r", layers.TargetsFrag);
				registerTypeLayer(ButtonType::SlideL, "target_slide26b_l", layers.TargetsFragHit);
				registerTypeLayer(ButtonType::SlideR, "target_slide26b_r", layers.TargetsFragHit);

				registerTypeLayer(ButtonType::Triangle, "target_sankaku_sync", layers.TargetsSync);
				registerTypeLayer(ButtonType::Square, "target_shikaku_sync", layers.TargetsSync);
				registerTypeLayer(ButtonType::Cross, "target_batsu_sync", layers.TargetsSync);
				registerTypeLayer(ButtonType::Circle, "target_maru_sync", layers.TargetsSync);

				registerTypeLayer(ButtonType::SlideL, "target_slide18_l_sync", layers.TargetsSync);
				registerTypeLayer(ButtonType::SlideR, "target_slide18_r_sync", layers.TargetsSync);
				registerTypeLayer(ButtonType::SlideL, "target_slide26_l_sync", layers.TargetsFragSync);
				registerTypeLayer(ButtonType::SlideR, "target_slide26_r_sync", layers.TargetsFragSync);

				registerTypeLayer(ButtonType::Triangle, "target_sankaku_hold", layers.TargetsHold);
				registerTypeLayer(ButtonType::Square, "target_shikaku_hold", layers.TargetsHold);
				registerTypeLayer(ButtonType::Cross, "target_batsu_hold", layers.TargetsHold);
				registerTypeLayer(ButtonType::Circle, "target_maru_hold", layers.TargetsHold);

				registerTypeLayer(ButtonType::Triangle, "target_sankaku_synchold", layers.TargetsSyncHold);
				registerTypeLayer(ButtonType::Square, "target_shikaku_synchold", layers.TargetsSyncHold);
				registerTypeLayer(ButtonType::Cross, "target_batsu_synchold", layers.TargetsSyncHold);
				registerTypeLayer(ButtonType::Circle, "target_maru_synchold", layers.TargetsSyncHold);

				registerTypeLayer(ButtonType::Triangle, "button_sankaku", layers.Buttons);
				registerTypeLayer(ButtonType::Square, "button_shikaku", layers.Buttons);
				registerTypeLayer(ButtonType::Cross, "button_batsu", layers.Buttons);
				registerTypeLayer(ButtonType::Circle, "button_maru", layers.Buttons);
				registerTypeLayer(ButtonType::SlideL, "button_slide18_l", layers.Buttons);
				registerTypeLayer(ButtonType::SlideR, "button_slide18_r", layers.Buttons);

				registerTypeLayer(ButtonType::SlideL, "button_slide25_l", layers.ButtonsFrag);
				registerTypeLayer(ButtonType::SlideR, "button_slide25_r", layers.ButtonsFrag);

				registerTypeLayer(ButtonType::Triangle, "button_sankaku_sync", layers.ButtonsSync);
				registerTypeLayer(ButtonType::Square, "button_shikaku_sync", layers.ButtonsSync);
				registerTypeLayer(ButtonType::Cross, "button_batsu_sync", layers.ButtonsSync);
				registerTypeLayer(ButtonType::Circle, "button_maru_sync", layers.ButtonsSync);
				registerTypeLayer(ButtonType::SlideL, "button_slide18_l_sync", layers.ButtonsSync);
				registerTypeLayer(ButtonType::SlideR, "button_slide18_r_sync", layers.ButtonsSync);

				registerTypeLayer(ButtonType::SlideL, "button_slide25_l_sync", layers.ButtonsFragSync);
				registerTypeLayer(ButtonType::SlideR, "button_slide25_r_sync", layers.ButtonsFragSync);

				if (!aetGameCommon->GetScenes().empty())
				{
					for (auto& video : aetGameCommon->GetScenes().front()->Videos)
					{
						if (video->Sources.size() == 1 && video->Sources.front().Name == "GAM_CMN_TARGET_HAND")
							layers.TargetHandVideo = video;
					}
				}
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

		// HACK: This is an incredibly hacky solution but is far more simple than creating entire layer + comp copies for each variation
		void PushSetScaleKeyFrames(const Aet::Layer& baseLayer) const
		{
			auto push = [&](auto& keys)
			{
				const auto staticScale = keys.empty() ? 0.0f : keys.front().Value;

				std::for_each(keys.begin(), keys.end(), [&](auto& key)
				{
					tempKeyFrameBackupStack.push(key.Value);
					key.Value = staticScale;
				});
			};

			for (auto& item : baseLayer.GetCompItem()->GetLayers())
			{
				push(item->LayerVideo->Transform.Scale.X.Keys);
				push(item->LayerVideo->Transform.Scale.Y.Keys);
			}
		}
		void PopRestoreScaleKeyFrames(const Aet::Layer& baseLayer) const
		{
			auto pop = [&](auto& keys)
			{
				std::for_each(keys.rbegin(), keys.rend(), [&](auto& key)
				{
					key.Value = tempKeyFrameBackupStack.top();
					tempKeyFrameBackupStack.pop();
				});
			};

			for (auto& item : baseLayer.GetCompItem()->GetLayers())
			{
				pop(item->LayerVideo->Transform.Scale.Y.Keys);
				pop(item->LayerVideo->Transform.Scale.X.Keys);
			}
		}

		void PushHideHandLayers(const Aet::Layer& baseLayer) const
		{
			auto push = [&](Aet::Layer& layer)
			{
				if (layer.GetVideoItem().get() == layers.TargetHandVideo.get())
				{
					tempLayerVisibleBackupStack.push(layer.Flags.VideoActive);
					layer.Flags.VideoActive = false;
				}
			};

			std::for_each(baseLayer.GetCompItem()->GetLayers().begin(), baseLayer.GetCompItem()->GetLayers().end(), [&](auto& layer)
			{
				if (auto& comp = layer->GetCompItem(); comp != nullptr)
					std::for_each(comp->GetLayers().begin(), comp->GetLayers().end(), [&](auto& l) { push(*l); });
				else
					push(*layer);
			});
		}
		void PopRestoreHandLayers(const Aet::Layer& baseLayer) const
		{
			auto pop = [&](Aet::Layer& layer)
			{
				if (layer.GetVideoItem().get() == layers.TargetHandVideo.get())
				{
					layer.Flags.VideoActive = tempLayerVisibleBackupStack.top();
					tempLayerVisibleBackupStack.pop();
				}
			};

			std::for_each(baseLayer.GetCompItem()->GetLayers().rbegin(), baseLayer.GetCompItem()->GetLayers().rend(), [&](auto& layer)
			{
				if (auto& comp = layer->GetCompItem(); comp != nullptr)
					std::for_each(comp->GetLayers().rbegin(), comp->GetLayers().rend(), [&](auto& l) { pop(*l); });
				else
					pop(*layer);
			});
		}

		void DrawTarget(Render::Renderer2D& renderer, const TargetDrawData& data) const
		{
			// TODO: Handle all other target types
			const auto* layer = (data.Sync ? layers.TargetsSync : layers.Targets)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			if (data.NoScale) PushSetScaleKeyFrames(*layer);
			if (data.NoHand) PushHideHandLayers(*layer);

			auto transform = Transform2D(data.Position);
			if (data.NoHand)
			{
				// DEBUG: To make the hand no hand difference clearer for now while buttons aren't yet implemented
				transform.Opacity = 0.5f;
			}

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, data.Progress * layerFrameScale, transform);

			if (data.NoHand) PopRestoreHandLayers(*layer);
			if (data.NoScale) PopRestoreScaleKeyFrames(*layer);
		}

		void DrawButton(Render::Renderer2D& renderer, const ButtonDrawData& data) const
		{
			// TODO: Handle all other button types
			const auto* layer = (data.Sync ? layers.ButtonsSync : layers.Buttons)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, data.Progress * layerFrameScale, Transform2D(data.Position));
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

		mutable std::stack<f32> tempKeyFrameBackupStack;
		mutable std::stack<bool> tempLayerVisibleBackupStack;

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

			std::array<std::shared_ptr<Aet::Layer>, EnumCount<ButtonType>()>
				Targets,
				TargetsFrag,
				TargetsFragHit,
				TargetsSync,
				TargetsFragSync,
				TargetsHold,
				TargetsSyncHold,
				Buttons,
				ButtonsFrag,
				ButtonsSync,
				ButtonsFragSync;

			std::shared_ptr<Aet::Video> TargetHandVideo;

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

	void TargetRenderHelper::DrawTarget(Render::Renderer2D& renderer, const TargetDrawData& data) const
	{
		impl->DrawTarget(renderer, data);
	}

	void TargetRenderHelper::DrawButton(Render::Renderer2D& renderer, const ButtonDrawData& data) const
	{
		impl->DrawButton(renderer, data);
	}
}

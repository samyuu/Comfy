#include "TargetRenderHelper.h"
#include "Editor/Chart/TargetPropertyRules.h"
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
		void UpdateAsyncLoading(Render::Renderer2D& renderer)
		{
			auto findLayer = [](AetSet& aetSet, std::string_view layerName, size_t sceneIndex = 0)
			{
				return InBounds(sceneIndex, aetSet.GetScenes()) ? aetSet.GetScenes()[sceneIndex]->FindLayer(layerName) : nullptr;
			};

			auto findSprite = [](SprSet& sprSet, std::string_view spriteName)
			{
				const auto found = FindIfOrNull(sprSet.Sprites, [&](const auto& spr) {return spr.Name == spriteName; });
				return (found != nullptr && InBounds(found->TextureIndex, sprSet.TexSet.Textures)) ? found : nullptr;
			};

			if (GetFutureIfReady(aetGameCommonFuture, aetGameCommon) && aetGameCommon != nullptr)
			{
				layers.FrameUpFT = findLayer(*aetGameCommon, "frame_up_ft");
				layers.FrameUpT = findLayer(*aetGameCommon, "frame_up_t");
				layers.FrameUpF = findLayer(*aetGameCommon, "frame_up_f");
				layers.FrameBottomFT = findLayer(*aetGameCommon, "frame_bottom_ft");
				layers.FrameBottomT = findLayer(*aetGameCommon, "frame_bottom_t");
				layers.FrameBottomF = findLayer(*aetGameCommon, "frame_bottom_f");
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

				registerTypeLayer(ButtonType::Triangle, "shadow_sankaku", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::Square, "shadow_shikaku", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::Cross, "shadow_batsu", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::Circle, "shadow_maru", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::SlideL, "shadow_slide_l18_l", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::SlideR, "shadow_slide_l18_r", layers.ButtonShadowsBlack);
				registerTypeLayer(ButtonType::SlideL, "shadow_slide_s25_l", layers.ButtonShadowsBlackFrag);
				registerTypeLayer(ButtonType::SlideR, "shadow_slide_s25_r", layers.ButtonShadowsBlackFrag);

				registerTypeLayer(ButtonType::Triangle, "white_sankaku", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::Square, "white_shikaku", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::Cross, "white_batsu", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::Circle, "white_maru", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::SlideL, "white_slide_l18_l", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::SlideR, "white_slide_l18_r", layers.ButtonShadowsWhite);
				registerTypeLayer(ButtonType::SlideL, "white_slide_s25_l", layers.ButtonShadowsWhiteFrag);
				registerTypeLayer(ButtonType::SlideR, "white_slide_s25_r", layers.ButtonShadowsWhiteFrag);

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
				renderer.UploadToGPUFreeCPUMemory(*sprGameCommon);
				sprites.ButtonTrail = findSprite(*sprGameCommon, "KISEKI");
				sprites.ButtonSyncLine = findSprite(*sprGameCommon, "KISEKI_SYNC");
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
				renderer.UploadToGPUFreeCPUMemory(*sprGame);
				sprites.PracticeNumbers = findSprite(*sprGame, "PRC_NUM24X36");
			}

			if (GetFutureIfReady(sprFont36Future, sprFont36) && sprFont36 != nullptr)
			{
				renderer.UploadToGPUFreeCPUMemory(*sprFont36);
			}

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

		void SetAetSprGetter(Render::Renderer2D& renderer)
		{
			// TODO: Find a better solution for this, maybe store tex shared_ptr + spr as mutable VideoSource member (?)
			renderer.Aet().SetSprGetter([this](const Aet::VideoSource& source) -> Render::TexSprView
			{
				if (auto result = Render::SprSetNameStringSprGetter(source, sprGameCommon.get()); result)
					return result;

				if (auto result = Render::SprSetNameStringSprGetter(source, sprGame.get()); result)
					return result;

				return Render::NullSprGetter(source);
			});
		}

		void DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const
		{
			if (aetGameCommon == nullptr || sprGameCommon == nullptr || aetGame == nullptr || sprGame == nullptr || sprFont36 == nullptr)
				return;

			const auto playbackFrame = hud.PlaybackTime.ToFrames();
			TryDrawLayer(renderer, layers.FrameUpT, playbackFrame);
			TryDrawLayer(renderer, layers.FrameBottomT, playbackFrame);
			TryDrawLayer(renderer, layers.LifeGauge, playbackFrame);
			TryDrawLayer(renderer, layers.SongIconLoop, playbackFrame);
			TryDrawLayer(renderer, layers.LevelInfoHard, playbackFrame);

			TryDrawLayer(renderer, layers.PracticeGaugeBase, playbackFrame);
			DrawHUDPracitceTime(renderer, hud.PlaybackTime);

			const auto progress = std::clamp(static_cast<f32>(hud.PlaybackTime / hud.Duration), 0.0f, 1.0f);
			const auto progressOnStart = std::clamp(static_cast<f32>(hud.PlaybackTimeOnStart / hud.Duration), 0.0f, 1.0f);

			TryDrawLayer(renderer, layers.PracticeGaugeTime, progress * 100.0f);
			TryDrawLayer(renderer, layers.PracticeGaugeBorderRestart, (hud.IsPlayback ? progressOnStart : progress) * 100.0f);

			if (const auto* font = GetFont36(); font != nullptr && !hud.SongTitle.empty())
				renderer.Font().DrawBorder(*font, hud.SongTitle, TryGetTransform(layers.SongTitle, playbackFrame));
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

		void DrawTarget(Render::Renderer2D& renderer, const TargetData& data) const
		{
			auto getLayerArray = [this](const TargetData& data) -> auto&
			{
				if (data.Fragment)
					return data.FragmentHit ? layers.TargetsFragHit : data.Sync ? layers.TargetsFragSync : layers.TargetsFrag;

				if (data.Sync)
					return data.HoldText ? layers.TargetsSyncHold : layers.TargetsSync;

				if (data.HoldText)
					return layers.TargetsHold;

				return layers.Targets;
			};

			const auto* layer = getLayerArray(data)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			if (data.NoScale) PushSetScaleKeyFrames(*layer);
			if (data.NoHand) PushHideHandLayers(*layer);

			auto transform = Transform2D(data.Position);
			if (data.Transparent)
				transform.Opacity = 0.5f;

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, data.Progress * layerFrameScale, transform);

			if (data.NoHand) PopRestoreHandLayers(*layer);
			if (data.NoScale) PopRestoreScaleKeyFrames(*layer);
		}

		void DrawButton(Render::Renderer2D& renderer, const ButtonData& data) const
		{
			auto getLayerArray = [this](const ButtonData& data) -> auto&
			{
				if (data.Fragment)
					return data.Sync ? layers.ButtonsFragSync : layers.ButtonsFrag;

				return data.Sync ? layers.ButtonsSync : layers.Buttons;
			};

			const auto* layer = getLayerArray(data)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, (data.Progress * layerFrameScale), Transform2D(data.Position));
		}

		void DrawButtonShadow(Render::Renderer2D& renderer, const ButtonData& data) const
		{
			const auto* layer =
				(data.Shadow == ButtonShadowType::Black) ? (data.Fragment ? layers.ButtonShadowsBlackFrag : layers.ButtonShadowsBlack)[static_cast<size_t>(data.Type)].get() :
				(data.Shadow == ButtonShadowType::White) ? (data.Fragment ? layers.ButtonShadowsWhiteFrag : layers.ButtonShadowsWhite)[static_cast<size_t>(data.Type)].get() : nullptr;

			if (layer == nullptr)
				return;

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, (data.Progress * layerFrameScale), Transform2D(data.Position));
		}

		void DrawButtonTrail(Render::Renderer2D& renderer, const ButtonTrailData& data) const
		{
			const auto trail = GetButtonTrailSprite();
			if (!trail)
				return;

			constexpr i32 segmentCount = 40;

			constexpr i32 verticesPerSegment = 2;
			constexpr i32 vertexCount = (segmentCount * verticesPerSegment);

			constexpr auto texSizeScale = vec2(8.0f, 0.25f);
			constexpr auto texCoordOffsetU = 0.35f;

			static constexpr std::array<u8, segmentCount> trailSegmentAlphaValues =
			{
				0x00, 0x00, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
				0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F,
				0x7F, 0x7F, 0x7F, 0x7F, 0x72, 0x72, 0x65, 0x65,
				0x59, 0x59, 0x4C, 0x4C, 0x3F, 0x3F, 0x32, 0x32,
				0x26, 0x26, 0x19, 0x19, 0x0C, 0x0C, 0x00, 0x00,
			};

			static constexpr std::array<vec3, EnumCount<ButtonType>()> trailButtonColors =
			{
				vec3(0.95f, 1.00f, 0.69f),
				vec3(1.00f, 0.81f, 1.00f),
				vec3(0.71f, 1.00f, 1.00f),
				vec3(0.93f, 0.27f, 0.29f),
				vec3(1.00f, 1.00f, 0.78f),
				vec3(1.00f, 1.00f, 0.78f),
			};

			static constexpr vec3 chanceTrailColor = vec3(1.0f, 1.0f, 1.0f);

			// NOTE: This single sprite consists of two "sub sprites" for chance time and normal button trails
			const auto texSize = trail.Spr->GetSize() * texSizeScale;
			const auto sprSize = vec2(texSize.x, texSize.y / 2);

			const f32 progressPerSegment = (data.ProgressEnd - data.ProgressStart) / static_cast<f32>(segmentCount);
			std::array<Render::PositionTextureColorVertex, vertexCount> vertices;

			std::array<vec2, segmentCount> segmentPositions;
			for (i32 i = 0; i < segmentCount; i++)
				segmentPositions[i] = GetButtonPathSinePoint(glm::min(data.ProgressStart + (static_cast<f32>(i) * progressPerSegment), 1.0f), data.Properties);

			for (i32 segment = 0, vertex = 0; segment < segmentCount; segment++)
			{
				const auto getNormal = [](vec2 v) { return vec2(v.y, -v.x); };

				const auto normal =
					(segment < 1) ? glm::normalize(getNormal(segmentPositions[segment + 1] - segmentPositions[segment])) :
					(segment >= segmentCount - 1) ? glm::normalize(getNormal(segmentPositions[segment] - segmentPositions[segment - 1])) :
					glm::normalize(getNormal(segmentPositions[segment] - segmentPositions[segment - 1]) + getNormal(segmentPositions[segment + 1] - segmentPositions[segment]));

				vertices[vertex++].Position = vec2(segmentPositions[segment] + normal * +sprSize.y);
				vertices[vertex++].Position = vec2(segmentPositions[segment] + normal * -sprSize.y);
			}

			const auto texCoordsV = std::array { vec2(0.0f, 0.5f), vec2(0.5, 1.0f) }[data.Chance];
			f32 coordDistU = (data.Progress + texCoordOffsetU);

			for (i32 vertex = 0; vertex < vertexCount; vertex += verticesPerSegment)
			{
				if (vertex > 0)
					coordDistU += (glm::distance(vertices[vertex - 1].Position, vertices[vertex].Position) / sprSize.x) * -0.5f;

				vertices[vertex + 0].TextureCoordinates = vec2(coordDistU, texCoordsV.x);
				vertices[vertex + 1].TextureCoordinates = vec2(coordDistU, texCoordsV.y);
			}

			const auto buttonColor = data.Chance ? chanceTrailColor : trailButtonColors[static_cast<size_t>(data.Type)];
			for (i32 segment = 0, vertex = 0; segment < segmentCount; segment++)
			{
				const auto segmentAlpha = trailSegmentAlphaValues[segment] * (1.0f / static_cast<f32>(std::numeric_limits<u8>::max()));
				const auto segmentColor = vec4(buttonColor, segmentAlpha);

				vertices[vertex++].Color = segmentColor;
				vertices[vertex++].Color = segmentColor;
			}

			renderer.DrawVertices(
				vertices.data(),
				vertices.size(),
				Render::TexSamplerView(trail.Tex, TextureAddressMode::WrapRepeat, TextureAddressMode::ClampBorder, TextureFilter::Linear),
				AetBlendMode::Normal,
				PrimitiveType::TriangleStrip);
		}

		void DrawButtonPairSyncLines(Render::Renderer2D& renderer, const ButtonSyncLineData& data) const
		{
			if (data.SyncPairCount < 2)
				return;

			const auto syncLine = GetButtonSyncLineSprite();
			if (!syncLine)
				return;

			if (data.SyncPairCount == 2)
			{
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[0], data.ButtonPositions[1], data.Progress);
			}
			else if (data.SyncPairCount == 3)
			{
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[0], data.ButtonPositions[1], data.Progress);
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[1], data.ButtonPositions[2], data.Progress);
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[2], data.ButtonPositions[0], data.Progress);
			}
			else if (data.SyncPairCount == 4)
			{
				const auto& targets = data.TargetPositions;
				const auto& buttons = data.ButtonPositions;

				const auto centroid = (targets[0] + targets[1] + targets[2] + targets[3]) / 4.0f;
				const auto centroidAngle = [centroid](const auto& v) { return glm::atan(centroid.y - v.y, centroid.x - v.x); };

				std::array<const vec2*, 4> sortedTargets;
				for (size_t i = 0; i < sortedTargets.size(); i++)
					sortedTargets[i] = &targets[i];

				std::sort(sortedTargets.begin(), sortedTargets.end(), [&](auto& a, auto& b) { return centroidAngle(*a) > centroidAngle(*b); });

				std::array<vec2, 4> sortedButtons;
				for (size_t i = 0; i < 4; i++)
					sortedButtons[i] = buttons[std::distance(&targets[0], sortedTargets[i])];

				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[0], sortedButtons[1], data.Progress);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[1], sortedButtons[2], data.Progress);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[2], sortedButtons[3], data.Progress);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[3], sortedButtons[0], data.Progress);
			}
		}

		void DrawSingleButtonSyncLine(Render::Renderer2D& renderer, Render::TexSprView syncLine, vec2 start, vec2 end, f32 progress) const
		{
			assert(syncLine);
			std::array<Render::RenderCommand2D, 2> commands;

			const auto spriteSize = syncLine.Spr->GetSize();
			const auto spriteSizeHalf = spriteSize * 0.5f;

			const auto distance = glm::distance(start, end);
			const auto distanceHalf = distance * 0.5f;

			commands[0].TexView.Texture = syncLine.Tex;
			commands[0].TexView.AddressU = TextureAddressMode::WrapRepeat;
			commands[1].TexView = commands[0].TexView;

			constexpr auto transparentWhite = vec4(1.0f, 1.0f, 1.0f, 0.0f);
			commands[0].CornerColors[0] = transparentWhite;
			commands[0].CornerColors[2] = transparentWhite;
			commands[1].CornerColors[1] = transparentWhite;
			commands[1].CornerColors[3] = transparentWhite;

			commands[0].Position = start;
			commands[1].Position = (end + start) / 2.0f;

			commands[0].Origin = vec2(0.0f, spriteSizeHalf.y);
			commands[1].Origin = commands[0].Origin;

			auto vecToAngle = [](vec2 vec) { return glm::degrees(glm::atan(vec.y, vec.x)); };
			commands[0].Rotation = vecToAngle(end - start);
			commands[1].Rotation = commands[0].Rotation;

			commands[0].Scale = vec2((distance / spriteSize.x), -0.5f);
			commands[1].Scale = commands[0].Scale;

			constexpr auto textureWidthsToScrollPerFullProgressCycle = 1.0f;
			const auto scroll = (progress * spriteSize.x * textureWidthsToScrollPerFullProgressCycle);

			commands[0].SourceRegion = vec4(scroll, 0.0f, spriteSizeHalf.x, spriteSize.y);
			commands[1].SourceRegion = vec4(scroll + spriteSizeHalf.x, 0.0f, spriteSizeHalf.x, spriteSize.y);

			for (const auto& command : commands)
				renderer.Draw(command);
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

			const auto playbackTimeRound = TimeSpan::RoundToMilliseconds(playbackTime);
			const auto totalSeconds = playbackTimeRound.TotalSeconds();
			const auto adjustedTime = (glm::isnan(totalSeconds) || glm::isinf(totalSeconds)) ? minValidTime : playbackTimeRound;
			const auto clampedSeconds = std::clamp(adjustedTime, minValidTime, maxValidTime).TotalSeconds();

			const auto fractionMin = glm::floor(glm::mod(clampedSeconds, 3600.0) / 60.0);
			const auto fractionSec = glm::mod(clampedSeconds, 60.0);
			const auto fractionMS = (fractionSec - glm::floor(fractionSec)) * 100.0;

			char formatBuffer[16];
			sprintf_s(formatBuffer, "%01d:%02d.%02d",
				static_cast<i32>(fractionMin),
				static_cast<i32>(fractionSec),
				static_cast<i32>(fractionMS));

			const auto formattedTime = std::string_view(formatBuffer, std::size(formatBuffer));
			renderer.Font().Draw(*font, formattedTime.substr(0, 1), TryGetTransform(layers.PracticeGaugeBaseNumPointMin, 0.0f));
			renderer.Font().Draw(*font, formattedTime.substr(2, 2), TryGetTransform(layers.PracticeGaugeBaseNumPointSec, 0.0f));
			renderer.Font().Draw(*font, formattedTime.substr(5, 2), TryGetTransform(layers.PracticeGaugeBaseNumPointMS, 0.0f));
		}

		void TryDrawLayer(Render::Renderer2D& renderer, const std::shared_ptr<Aet::Layer>& layer, frame_t frame) const
		{
			if (layer != nullptr) { renderer.Aet().DrawLayerLooped(*layer, frame); }
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

		Render::TexSprView GetButtonTrailSprite() const
		{
			return (sprites.ButtonTrail == nullptr) ?
				Render::TexSprView { nullptr, nullptr } :
				Render::TexSprView { sprGameCommon->TexSet.Textures[sprites.ButtonTrail->TextureIndex].get(), sprites.ButtonTrail };
		}

		Render::TexSprView GetButtonSyncLineSprite() const
		{
			return (sprites.ButtonSyncLine == nullptr) ?
				Render::TexSprView { nullptr, nullptr } :
				Render::TexSprView { sprGameCommon->TexSet.Textures[sprites.ButtonSyncLine->TextureIndex].get(), sprites.ButtonSyncLine };
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
				FrameUpFT,
				FrameUpT,
				FrameUpF,
				FrameBottomFT,
				FrameBottomT,
				FrameBottomF,
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
				ButtonsFragSync,
				ButtonShadowsBlack,
				ButtonShadowsBlackFrag,
				ButtonShadowsWhite,
				ButtonShadowsWhiteFrag;

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

			Spr* ButtonTrail;
			Spr* ButtonSyncLine;
		} sprites = {};
	};

	TargetRenderHelper::TargetRenderHelper() : impl(std::make_unique<Impl>())
	{
	}

	TargetRenderHelper::~TargetRenderHelper()
	{
	}

	void TargetRenderHelper::UpdateAsyncLoading(Render::Renderer2D& renderer)
	{
		impl->UpdateAsyncLoading(renderer);
	}

	void TargetRenderHelper::SetAetSprGetter(Render::Renderer2D& renderer)
	{
		impl->SetAetSprGetter(renderer);
	}

	void TargetRenderHelper::DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const
	{
		impl->DrawHUD(renderer, hud);
	}

	void TargetRenderHelper::DrawTarget(Render::Renderer2D& renderer, const TargetData& data) const
	{
		impl->DrawTarget(renderer, data);
	}

	void TargetRenderHelper::DrawButton(Render::Renderer2D& renderer, const ButtonData& data) const
	{
		impl->DrawButton(renderer, data);
	}

	void TargetRenderHelper::DrawButtonShadow(Render::Renderer2D& renderer, const ButtonData& data) const
	{
		impl->DrawButtonShadow(renderer, data);
	}

	void TargetRenderHelper::DrawButtonTrail(Render::Renderer2D& renderer, const ButtonTrailData& data) const
	{
		impl->DrawButtonTrail(renderer, data);
	}

	void TargetRenderHelper::DrawButtonPairSyncLines(Render::Renderer2D& renderer, const ButtonSyncLineData& data) const
	{
		impl->DrawButtonPairSyncLines(renderer, data);
	}
}

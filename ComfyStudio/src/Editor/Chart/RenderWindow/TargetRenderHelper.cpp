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
	enum class ComboDigitPlace : u8
	{
		Ones,
		Tens,
		Hundreds,
		HundredsExact,
		Thousands,
		Count
	};

	constexpr ComboDigitPlace GetComboDigitPlace(i32 comboCount)
	{
		if (comboCount < 10)
			return ComboDigitPlace::Ones;
		else if (comboCount < 100)
			return ComboDigitPlace::Tens;
		else if (comboCount < 1000)
			return ((comboCount % 100) == 0) ? ComboDigitPlace::HundredsExact : ComboDigitPlace::Hundreds;
		else
			return ComboDigitPlace::Thousands;
	}

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

			auto findVideo = [](AetSet& aetSet, std::string_view videoName, size_t sceneIndex = 0)
			{
				auto found = !InBounds(sceneIndex, aetSet.GetScenes()) ? nullptr : FindIfOrNull(aetSet.GetScenes()[sceneIndex]->Videos, [&](const auto& video)
				{
					return (video->Sources.size() == 1) && (video->Sources.front().Name == videoName);
				});
				return (found != nullptr) ? *found : nullptr;
			};

			auto findSprite = [](SprSet& sprSet, std::string_view spriteName)
			{
				const auto found = FindIfOrNull(sprSet.Sprites, [&](const auto& spr) { return spr.Name == spriteName; });
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
				layers.LifeGaugeInsurance = findLayer(*aetGameCommon, "life_gauge_insurance");
				layers.SongEnergyBase = findLayer(*aetGameCommon, "song_energy_base_t");
				layers.SongEnergyNormal = findLayer(*aetGameCommon, "song_energy_normal");
				layers.SongIconLoop = findLayer(*aetGameCommon, "song_icon_loop");
				layers.LevelInfoEasy = findLayer(*aetGameCommon, "level_info_easy");
				layers.LevelInfoNormal = findLayer(*aetGameCommon, "level_info_normal");
				layers.LevelInfoHard = findLayer(*aetGameCommon, "level_info_hard");
				layers.LevelInfoExtreme = findLayer(*aetGameCommon, "level_info_extreme");
				layers.LevelInfoExExtreme = findLayer(*aetGameCommon, "level_info_extreme_extra");
				layers.SongTitle = findLayer(*aetGameCommon, "p_song_title_lt");

				layers.SyncInfoSingle = findLayer(*aetGameCommon, "sync_info_single");
				layers.SyncInfoDouble = findLayer(*aetGameCommon, "sync_info_double");
				layers.SyncInfoDoubleAdd = findLayer(*aetGameCommon, "sync_info_double_add");
				layers.SyncInfoTriple = findLayer(*aetGameCommon, "sync_info_triple");
				layers.SyncInfoTripleAdd = findLayer(*aetGameCommon, "sync_info_triple_add");
				layers.SyncInfoQuadruple = findLayer(*aetGameCommon, "sync_info_quadruple");
				layers.SyncInfoQuadrupleAdd = findLayer(*aetGameCommon, "sync_info_quadruple_add");
				layers.SyncInfoMaxAdd = findLayer(*aetGameCommon, "sync_info_max_add");

				if (auto* layer = layers.SyncInfoQuadruple.get(); layer != nullptr)
				{
					auto findLayerVideo = [&](std::string_view layerName)
					{
						auto* videoLayer = layer->GetCompItem()->FindLayer(layerName).get();
						return (videoLayer != nullptr) ? videoLayer->GetVideoItem() : nullptr;
					};
					videos.SyncInfoButtonIconPlaceholders[0] = findLayerVideo("p_sync_icon01_c");
					videos.SyncInfoButtonIconPlaceholders[1] = findLayerVideo("p_sync_icon02_c");
					videos.SyncInfoButtonIconPlaceholders[2] = findLayerVideo("p_sync_icon03_c");
					videos.SyncInfoButtonIconPlaceholders[3] = findLayerVideo("p_sync_icon04_c");

					videos.SyncInfoScoreDigitPlaceholders[0] = findLayerVideo("p_sync_score_00000001_c");
					videos.SyncInfoScoreDigitPlaceholders[1] = findLayerVideo("p_sync_score_00000010_c");
					videos.SyncInfoScoreDigitPlaceholders[2] = findLayerVideo("p_sync_score_00000100_c");
					videos.SyncInfoScoreDigitPlaceholders[3] = findLayerVideo("p_sync_score_00001000_c");
					videos.SyncInfoScoreDigitPlaceholders[4] = findLayerVideo("p_sync_score_00010000_c");
					videos.SyncInfoScoreDigitPlaceholders[5] = findLayerVideo("p_sync_score_00100000_c");
					videos.SyncInfoScoreDigitPlaceholders[6] = findLayerVideo("p_sync_score_01000000_c");
					videos.SyncInfoScoreDigitPlaceholders[7] = findLayerVideo("p_sync_score_10000000_c");

					markers.SyncHoldInfo.LoopStart = TimeSpan::FromFrames(layer->FindMarkerFrame("loop_start").value_or(0.0f));
					markers.SyncHoldInfo.LoopEnd = TimeSpan::FromFrames(layer->FindMarkerFrame("loop_end").value_or(0.0f));
					markers.SyncHoldInfo.ChargeEnd = TimeSpan::FromFrames(layer->FindMarkerFrame("charge_end").value_or(0.0f));
				}

				if (auto* layer = layers.SyncInfoQuadrupleAdd.get(); layer != nullptr)
				{
					markers.SyncHoldInfo.LoopStartAdd = TimeSpan::FromFrames(layer->FindMarkerFrame("loop_start").value_or(0.0f));
				}

				if (auto* layer = layers.SyncInfoMaxAdd.get(); layer != nullptr)
				{
					markers.SyncHoldInfo.MaxLoopStart = TimeSpan::FromFrames(layer->FindMarkerFrame("loop_start").value_or(0.0f));
					markers.SyncHoldInfo.MaxChargeEnd = TimeSpan::FromFrames(layer->FindMarkerFrame("charge_end").value_or(0.0f));
					markers.SyncHoldInfo.MaxLoopEnd = TimeSpan::FromFrames(layer->FindMarkerFrame("loop_end").value_or(0.0f));
				}

				layers.TargetAppearEffect = findLayer(*aetGameCommon, "target_eff");

				layers.HitEffects[0] = findLayer(*aetGameCommon, "hit_eff00");
				layers.HitEffects[1] = findLayer(*aetGameCommon, "hit_eff01");
				layers.HitEffects[2] = findLayer(*aetGameCommon, "hit_eff02");
				layers.HitEffects[3] = findLayer(*aetGameCommon, "hit_eff03");
				layers.HitEffects[4] = findLayer(*aetGameCommon, "hit_eff04");
				layers.HitEffectsSlideL[0] = findLayer(*aetGameCommon, "hit_eff00_l");
				layers.HitEffectsSlideL[1] = findLayer(*aetGameCommon, "hit_eff01_l");
				layers.HitEffectsSlideR[0] = findLayer(*aetGameCommon, "hit_eff00_r");
				layers.HitEffectsSlideR[1] = findLayer(*aetGameCommon, "hit_eff01_r");
				layers.HitEffectsChainL[0] = findLayer(*aetGameCommon, "hit_eff_slide01_l");
				layers.HitEffectsChainR[0] = findLayer(*aetGameCommon, "hit_eff_slide01_r");

				layers.ValueTextCool[0] = findLayer(*aetGameCommon, "value_text_cool01");
				layers.ValueTextCool[1] = findLayer(*aetGameCommon, "value_text_cool02");
				layers.ValueTextCool[2] = findLayer(*aetGameCommon, "value_text_cool03");
				layers.ValueTextCool[3] = findLayer(*aetGameCommon, "value_text_cool04");

				layers.ValueTextFine[0] = findLayer(*aetGameCommon, "value_text_fine01");
				layers.ValueTextFine[1] = findLayer(*aetGameCommon, "value_text_fine02");
				layers.ValueTextFine[2] = findLayer(*aetGameCommon, "value_text_fine03");

				layers.ValueTextSad[0] = findLayer(*aetGameCommon, "value_text_sad");
				layers.ValueTextSafe[0] = findLayer(*aetGameCommon, "value_text_safe");
				layers.ValueTextWorst[0] = findLayer(*aetGameCommon, "value_text_worst");

				layers.ValueTextWrong[0] = findLayer(*aetGameCommon, "value_text_wrong01");
				layers.ValueTextWrong[1] = findLayer(*aetGameCommon, "value_text_wrong02");
				layers.ValueTextWrong[2] = findLayer(*aetGameCommon, "value_text_wrong03");
				layers.ValueTextWrong[3] = findLayer(*aetGameCommon, "value_text_wrong04");

				layers.ValueTextAlmost[0] = findLayer(*aetGameCommon, "value_text_almost01");
				layers.ValueTextAlmost[1] = findLayer(*aetGameCommon, "value_text_almost02");
				layers.ValueTextAlmost[2] = findLayer(*aetGameCommon, "value_text_almost03");
				layers.ValueTextAlmost[3] = findLayer(*aetGameCommon, "value_text_almost04");

				auto registerComboLayer = [&](ComboDigitPlace place, std::string_view layerName, auto& outArray)
				{
					outArray[static_cast<size_t>(place)] = findLayer(*aetGameCommon, layerName);
				};

				registerComboLayer(ComboDigitPlace::Ones, "combo_cool001", layers.ComboCoolEarly);
				registerComboLayer(ComboDigitPlace::Tens, "combo_cool010", layers.ComboCoolEarly);
				registerComboLayer(ComboDigitPlace::Hundreds, "combo_cool100", layers.ComboCoolEarly);
				registerComboLayer(ComboDigitPlace::HundredsExact, "combo_cool100_b", layers.ComboCoolEarly);
				registerComboLayer(ComboDigitPlace::Thousands, "combo_cool1000", layers.ComboCoolEarly);

				registerComboLayer(ComboDigitPlace::Ones, "combo_just_cool001", layers.ComboCoolJust);
				registerComboLayer(ComboDigitPlace::Tens, "combo_just_cool010", layers.ComboCoolJust);
				registerComboLayer(ComboDigitPlace::Hundreds, "combo_just_cool100", layers.ComboCoolJust);
				registerComboLayer(ComboDigitPlace::HundredsExact, "combo_just_cool100_b", layers.ComboCoolJust);
				registerComboLayer(ComboDigitPlace::Thousands, "combo_just_cool1000", layers.ComboCoolJust);

				registerComboLayer(ComboDigitPlace::Ones, "combo_tempo_cool001", layers.ComboCoolLate);
				registerComboLayer(ComboDigitPlace::Tens, "combo_tempo_cool010", layers.ComboCoolLate);
				registerComboLayer(ComboDigitPlace::Hundreds, "combo_tempo_cool100", layers.ComboCoolLate);
				registerComboLayer(ComboDigitPlace::HundredsExact, "combo_tempo_cool100_b", layers.ComboCoolLate);
				registerComboLayer(ComboDigitPlace::Thousands, "combo_tempo_cool1000", layers.ComboCoolLate);

				registerComboLayer(ComboDigitPlace::Ones, "combo_fine001", layers.ComboFineEarly);
				registerComboLayer(ComboDigitPlace::Tens, "combo_fine010", layers.ComboFineEarly);
				registerComboLayer(ComboDigitPlace::Hundreds, "combo_fine100", layers.ComboFineEarly);
				registerComboLayer(ComboDigitPlace::HundredsExact, "combo_fine100_b", layers.ComboFineEarly);
				registerComboLayer(ComboDigitPlace::Thousands, "combo_fine1000", layers.ComboFineEarly);

				registerComboLayer(ComboDigitPlace::Ones, "combo_tempo_fine001", layers.ComboFineLate);
				registerComboLayer(ComboDigitPlace::Tens, "combo_tempo_fine010", layers.ComboFineLate);
				registerComboLayer(ComboDigitPlace::Hundreds, "combo_tempo_fine100", layers.ComboFineLate);
				registerComboLayer(ComboDigitPlace::HundredsExact, "combo_tempo_fine100_b", layers.ComboFineLate);
				registerComboLayer(ComboDigitPlace::Thousands, "combo_tempo_fine1000", layers.ComboFineLate);

				if (auto* layer = layers.ComboCoolJust.back().get(); layer != nullptr)
				{
					auto findLayerVideo = [&](std::string_view layerName)
					{
						auto* videoLayer = layer->GetCompItem()->FindLayer(layerName).get();
						return (videoLayer != nullptr) ? videoLayer->GetVideoItem() : nullptr;
					};
					videos.ComboNumberDigitPlaceholders[0] = findLayerVideo("p_combo_num001_c");
					videos.ComboNumberDigitPlaceholders[1] = findLayerVideo("p_combo_num010_c");
					videos.ComboNumberDigitPlaceholders[2] = findLayerVideo("p_combo_num100_c");
					videos.ComboNumberDigitPlaceholders[3] = findLayerVideo("p_combo_num1000_c");
				}

				layers.MaxSlidePointOdd = findLayer(*aetGameCommon, "max_slide_point_odd");
				layers.MaxSlidePointEven = findLayer(*aetGameCommon, "max_slide_point_even");
				layers.ChancePointOdd = findLayer(*aetGameCommon, "chance_point_odd");
				layers.ChancePointEven = findLayer(*aetGameCommon, "chance_point_even");
				layers.MaxSlideNumbers[0] = findLayer(*aetGameCommon, "max_slide_num_00");
				layers.MaxSlideNumbers[1] = findLayer(*aetGameCommon, "max_slide_num_01");
				layers.MaxSlideNumbers[2] = findLayer(*aetGameCommon, "max_slide_num_02");
				layers.MaxSlideNumbers[3] = findLayer(*aetGameCommon, "max_slide_num_03");
				layers.MaxSlideNumbers[4] = findLayer(*aetGameCommon, "max_slide_num_04");
				layers.MaxSlideNumbers[5] = findLayer(*aetGameCommon, "max_slide_num_05");
				layers.MaxSlideNumbers[6] = findLayer(*aetGameCommon, "max_slide_num_06");
				layers.MaxSlideNumbers[7] = findLayer(*aetGameCommon, "max_slide_num_07");
				layers.MaxSlideNumbers[8] = findLayer(*aetGameCommon, "max_slide_num_08");
				layers.MaxSlideNumbers[9] = findLayer(*aetGameCommon, "max_slide_num_09");
				layers.MaxSlideNumbers[10] = findLayer(*aetGameCommon, "max_slide_num_plus");

				if (auto* layer = layers.ChancePointOdd.get(); layer != nullptr)
				{
					auto findLayerVideo = [&](std::string_view layerName)
					{
						auto* videoLayer = layer->GetCompItem()->FindLayer(layerName).get();
						return (videoLayer != nullptr) ? videoLayer->GetVideoItem() : nullptr;
					};
					videos.ChancePointDigitPlaceholders[0] = findLayerVideo("p_chance_point_num01_c");
					videos.ChancePointDigitPlaceholders[1] = findLayerVideo("p_chance_point_num02_c");
					videos.ChancePointDigitPlaceholders[2] = findLayerVideo("p_chance_point_num03_c");
					videos.ChancePointDigitPlaceholders[3] = findLayerVideo("p_chance_point_num04_c");
					videos.ChancePointDigitPlaceholders[4] = findLayerVideo("p_chance_point_num05_c");
					videos.ChancePointDigitPlaceholders[5] = findLayerVideo("p_chance_point_num06_c");
					videos.ChancePointDigitPlaceholders[6] = findLayerVideo("p_chance_point_num07_c");
				}

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
				registerTypeLayer(ButtonType::SlideL, "target_slide18b_l", layers.TargetsHit);
				registerTypeLayer(ButtonType::SlideR, "target_slide18b_r", layers.TargetsHit);
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

				registerTypeLayer(ButtonType::Triangle, "target_sp_sankaku", layers.TargetsChance);
				registerTypeLayer(ButtonType::Square, "target_sp_shikaku", layers.TargetsChance);
				registerTypeLayer(ButtonType::Cross, "target_sp_batsu", layers.TargetsChance);
				registerTypeLayer(ButtonType::Circle, "target_sp_maru", layers.TargetsChance);
				registerTypeLayer(ButtonType::SlideL, "target_sp_slide18_l", layers.TargetsChance);
				registerTypeLayer(ButtonType::SlideR, "target_sp_slide18_r", layers.TargetsChance);

				registerTypeLayer(ButtonType::Triangle, "target_sp_sankaku_hold", layers.TargetsChanceHold);
				registerTypeLayer(ButtonType::Square, "target_sp_shikaku_hold", layers.TargetsChanceHold);
				registerTypeLayer(ButtonType::Cross, "target_sp_batsu_hold", layers.TargetsChanceHold);
				registerTypeLayer(ButtonType::Circle, "target_sp_maru_hold", layers.TargetsChanceHold);
				registerTypeLayer(ButtonType::SlideL, "target_sp_slide18_l_hold", layers.TargetsChanceHold);
				registerTypeLayer(ButtonType::SlideR, "target_sp_slide18_r_hold", layers.TargetsChanceHold);

				registerTypeLayer(ButtonType::Triangle, "target_sp_sankaku_sync", layers.TargetsChanceSync);
				registerTypeLayer(ButtonType::Square, "target_sp_shikaku_sync", layers.TargetsChanceSync);
				registerTypeLayer(ButtonType::Cross, "target_sp_batsu_sync", layers.TargetsChanceSync);
				registerTypeLayer(ButtonType::Circle, "target_sp_maru_sync", layers.TargetsChanceSync);
				registerTypeLayer(ButtonType::SlideL, "target_sp_slide18_l_sync", layers.TargetsChanceSync);
				registerTypeLayer(ButtonType::SlideR, "target_sp_slide18_r_sync", layers.TargetsChanceSync);

				registerTypeLayer(ButtonType::Triangle, "target_sp_sankaku_synchold", layers.TargetsChanceSyncHold);
				registerTypeLayer(ButtonType::Square, "target_sp_shikaku_synchold", layers.TargetsChanceSyncHold);
				registerTypeLayer(ButtonType::Cross, "target_sp_batsu_synchold", layers.TargetsChanceSyncHold);
				registerTypeLayer(ButtonType::Circle, "target_sp_maru_synchold", layers.TargetsChanceSyncHold);
				registerTypeLayer(ButtonType::SlideL, "target_sp_slide18_l_synchold", layers.TargetsChanceSyncHold);
				registerTypeLayer(ButtonType::SlideR, "target_sp_slide18_r_synchold", layers.TargetsChanceSyncHold);

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

				videos.TargetHand = findVideo(*aetGameCommon, "GAM_CMN_TARGET_HAND");
			}

			if (GetFutureIfReady(sprGameCommonFuture, sprGameCommon) && sprGameCommon != nullptr)
			{
				renderer.UploadToGPUFreeCPUMemory(*sprGameCommon);
				sprites.ButtonTrail = findSprite(*sprGameCommon, "KISEKI");
				sprites.ButtonSyncLine = findSprite(*sprGameCommon, "KISEKI_SYNC");

				sprites.ComboNumbers[0] = findSprite(*sprGameCommon, "CMB_NUM_00");
				sprites.ComboNumbers[1] = findSprite(*sprGameCommon, "CMB_NUM_01");
				sprites.ComboNumbers[2] = findSprite(*sprGameCommon, "CMB_NUM_02");
				sprites.ComboNumbers[3] = findSprite(*sprGameCommon, "CMB_NUM_03");
				sprites.ComboNumbers[4] = findSprite(*sprGameCommon, "CMB_NUM_04");
				sprites.ComboNumbers[5] = findSprite(*sprGameCommon, "CMB_NUM_05");
				sprites.ComboNumbers[6] = findSprite(*sprGameCommon, "CMB_NUM_06");
				sprites.ComboNumbers[7] = findSprite(*sprGameCommon, "CMB_NUM_07");
				sprites.ComboNumbers[8] = findSprite(*sprGameCommon, "CMB_NUM_08");
				sprites.ComboNumbers[9] = findSprite(*sprGameCommon, "CMB_NUM_09");

				sprites.SyncInfoHoldScoreNumbers[0] = findSprite(*sprGameCommon, "SYNC_NUM_00");
				sprites.SyncInfoHoldScoreNumbers[1] = findSprite(*sprGameCommon, "SYNC_NUM_01");
				sprites.SyncInfoHoldScoreNumbers[2] = findSprite(*sprGameCommon, "SYNC_NUM_02");
				sprites.SyncInfoHoldScoreNumbers[3] = findSprite(*sprGameCommon, "SYNC_NUM_03");
				sprites.SyncInfoHoldScoreNumbers[4] = findSprite(*sprGameCommon, "SYNC_NUM_04");
				sprites.SyncInfoHoldScoreNumbers[5] = findSprite(*sprGameCommon, "SYNC_NUM_05");
				sprites.SyncInfoHoldScoreNumbers[6] = findSprite(*sprGameCommon, "SYNC_NUM_06");
				sprites.SyncInfoHoldScoreNumbers[7] = findSprite(*sprGameCommon, "SYNC_NUM_07");
				sprites.SyncInfoHoldScoreNumbers[8] = findSprite(*sprGameCommon, "SYNC_NUM_08");
				sprites.SyncInfoHoldScoreNumbers[9] = findSprite(*sprGameCommon, "SYNC_NUM_09");
				sprites.SyncInfoHoldScoreNumbers[10] = findSprite(*sprGameCommon, "SYNC_NUM_PLUS");

				sprites.SyncInfoHoldButtonIcons[static_cast<u8>(ButtonType::Triangle)] = findSprite(*sprGameCommon, "SYNC_ICON_SANKAKU");
				sprites.SyncInfoHoldButtonIcons[static_cast<u8>(ButtonType::Square)] = findSprite(*sprGameCommon, "SYNC_ICON_SHIKAKU");
				sprites.SyncInfoHoldButtonIcons[static_cast<u8>(ButtonType::Cross)] = findSprite(*sprGameCommon, "SYNC_ICON_BATSU");
				sprites.SyncInfoHoldButtonIcons[static_cast<u8>(ButtonType::Circle)] = findSprite(*sprGameCommon, "SYNC_ICON_MARU");

				for (auto& spr : sprGameCommon->Sprites)
					spr.Name = "GAM_CMN_" + spr.Name;
			}

			if (GetFutureIfReady(aetGameFuture, aetGame) && aetGame != nullptr)
			{
				layers.PracticeLevelInfoEasy = findLayer(*aetGame, "level_info_easy_prc");
				layers.PracticeLevelInfoNormal = findLayer(*aetGame, "level_info_normal_prc");
				layers.PracticeLevelInfoHard = findLayer(*aetGame, "level_info_hard_prc");
				layers.PracticeLevelInfoExtreme = findLayer(*aetGame, "level_info_extreme_prc");
				layers.PracticeLevelInfoExExtreme = findLayer(*aetGame, "level_info_extreme_extra_prc");
				layers.PracticeBackground = findLayer(*aetGame, "prc_bg");
				layers.PracticeGaugeBase = findLayer(*aetGame, "prc_gauge_base");
				layers.PracticeGaugeTime = findLayer(*aetGame, "prc_gauge_time");
				layers.PracticeGaugeBorderPlay = findLayer(*aetGame, "prc_gauge_border_play");
				layers.PracticeGaugeBorderRestart = findLayer(*aetGame, "prc_gauge_border_restart");
				layers.PracticeInfo = findLayer(*aetGame, "prc_info");

				if (layers.PracticeGaugeBase != nullptr && layers.PracticeGaugeBase->GetCompItem() != nullptr)
				{
					layers.PracticeGaugeBaseNumPointMin = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_min_rt");
					layers.PracticeGaugeBaseNumPointSec = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_sec_rt");
					layers.PracticeGaugeBaseNumPointMS = layers.PracticeGaugeBase->GetCompItem()->FindLayer("p_prc_num_frm_rt");
				}

				videos.PracticeBackgroundCommon = findVideo(*aetGame, "PS4_GAME_PRC_BG_CMN");
				videos.PracticeColorBlack = findVideo(*aetGame, "PS4_GAME_COL_BLACK");
				videos.PracticeLogoDummy = findVideo(*aetGame, "PS4_GAME_SONG_LOGO_DUMMY");
				videos.PracticeCoverDummy = findVideo(*aetGame, "PS4_GAME_SONG_JK_DUMMY");
				videos.PracticeCoverShadow = findVideo(*aetGame, "PS4_GAME_SONG_JK_SDW");
				videos.PracticeBackgroundDummy = findVideo(*aetGame, "PS4_GAME_SONG_BG_DUMMY");

				if (const auto& compItem = layers.PracticeBackground->GetCompItem(); compItem != nullptr)
				{
					for (auto& layer : compItem->GetLayers())
					{
						const auto* videoItem = layer->GetVideoItem().get();
						if (videoItem == videos.PracticeLogoDummy.get())
							videos.PracticeLogoDummyParentLayer = layer;
						else if (videoItem == videos.PracticeCoverDummy.get())
							videos.PracticeCoverDummyParentLayer = layer;
						else if (videoItem == videos.PracticeBackgroundDummy.get())
							videos.PracticeBackgroundDummyParentLayer = layer;
					}

					auto copyLayerVideoToRenderOverride = [](auto& layer) { if (layer != nullptr) { layer->RenderOverride.LayerVideo = *layer->LayerVideo; } };
					copyLayerVideoToRenderOverride(videos.PracticeLogoDummyParentLayer);
					copyLayerVideoToRenderOverride(videos.PracticeCoverDummyParentLayer);
					copyLayerVideoToRenderOverride(videos.PracticeBackgroundDummyParentLayer);
				}
			}

			if (GetFutureIfReady(sprGameFuture, sprGame) && sprGame != nullptr)
			{
				renderer.UploadToGPUFreeCPUMemory(*sprGame);
				sprites.PracticeNumbers = findSprite(*sprGame, "PRC_NUM24X36");

				for (auto& spr : sprGame->Sprites)
					spr.Name = "PS4_GAME_" + spr.Name;
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
				if (auto result = Render::SprSetNameStringSprGetterExact(source, sprGameCommon.get()); result)
					return result;

				if (auto result = Render::SprSetNameStringSprGetterExact(source, sprGame.get()); result)
					return result;

				return Render::NullSprGetter(source);
			});
		}

		void SetCenteredScaledRenderOverrideIf(const Aet::Layer& layer, const Aet::Video& videoItem, Render::TexSprView texSpr) const
		{
			if (texSpr)
			{
				layer.RenderOverride.UseLayerVideo = true;
				layer.RenderOverride.LayerVideoNoParentTransform = true;
				layer.RenderOverride.UseTexSpr = true;

				layer.RenderOverride.Tex = texSpr.Tex;
				layer.RenderOverride.Spr = texSpr.Spr;

				auto& oldTransform2D = layer.LayerVideo->Transform;
				auto& newTransform2D = layer.RenderOverride.LayerVideo.Transform;

				const auto oldSprSize = vec2(texSpr.Spr->GetSize());
				const auto newSprSize = vec2(videoItem.Size) * vec2(oldTransform2D.Scale.X.Keys[0].Value, oldTransform2D.Scale.Y.Keys[0].Value);

				newTransform2D.Origin.X.Keys[0].Value = (oldSprSize.x * 0.5f);
				newTransform2D.Origin.Y.Keys[0].Value = (oldSprSize.y * 0.5f);

				newTransform2D.Scale.X.Keys[0].Value = (newSprSize.x / oldSprSize.x);
				newTransform2D.Scale.Y.Keys[0].Value = (newSprSize.y / oldSprSize.y);
			}
			else
			{
				layer.RenderOverride.UseLayerVideo = false;
				layer.RenderOverride.UseTexSpr = false;
			}
		}

		void DrawBackground(Render::Renderer2D& renderer, const BackgroundData& background) const
		{
			if (aetGame == nullptr || sprGame == nullptr)
				return;

			if (layers.PracticeBackground == nullptr)
				return;

			// HACK: Same point as the other target layer manipulation hacks
			if (const auto& compItem = layers.PracticeBackground->GetCompItem(); compItem != nullptr)
			{
				for (auto& layer : compItem->GetLayers())
				{
					const auto* videoItem = layer->GetVideoItem().get();
					if (videoItem == videos.PracticeBackgroundCommon.get())
						layer->SetIsVisible(background.DrawGrid);
					else if (videoItem == videos.PracticeColorBlack.get())
						layer->SetIsVisible(background.DrawDim);
					else if (videoItem == videos.PracticeLogoDummy.get())
					{
						layer->SetIsVisible(background.DrawLogo);
						SetCenteredScaledRenderOverrideIf(*layer, *videoItem, background.LogoSprite);
					}
					else if (videoItem == videos.PracticeCoverDummy.get())
					{
						layer->SetIsVisible(background.DrawCover);
						SetCenteredScaledRenderOverrideIf(*layer, *videoItem, background.CoverSprite);
					}
					else if (videoItem == videos.PracticeCoverShadow.get())
						layer->SetIsVisible(background.DrawCover);
					else if (videoItem == videos.PracticeBackgroundDummy.get())
					{
						layer->SetIsVisible(background.DrawBackground);
						SetCenteredScaledRenderOverrideIf(*layer, *videoItem, background.BackgroundSprite);
					}
				}
			}

			const auto playbackFrame = background.PlaybackTime.ToFrames();
			TryDrawLayerLooped(renderer, layers.PracticeBackground, playbackFrame);
		}

		void DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const
		{
			if (aetGameCommon == nullptr || sprGameCommon == nullptr || aetGame == nullptr || sprGame == nullptr || sprFont36 == nullptr)
				return;

			const auto playbackFrame = hud.PlaybackTime.ToFrames();
			TryDrawLayerLooped(renderer, layers.FrameUpT, playbackFrame);
			TryDrawLayerLooped(renderer, layers.FrameBottomT, playbackFrame);
			TryDrawLayerLooped(renderer, layers.LifeGauge, playbackFrame);
			TryDrawLayerLooped(renderer, layers.LifeGaugeInsurance, playbackFrame);
			TryDrawLayerLooped(renderer, layers.SongIconLoop, playbackFrame);

			switch (hud.Difficulty)
			{
			case Difficulty::Easy: { TryDrawLayerLooped(renderer, layers.PracticeLevelInfoEasy, playbackFrame); break; }
			case Difficulty::Normal: { TryDrawLayerLooped(renderer, layers.PracticeLevelInfoNormal, playbackFrame); break; }
			case Difficulty::Hard: { TryDrawLayerLooped(renderer, layers.PracticeLevelInfoHard, playbackFrame); break; }
			case Difficulty::Extreme: { TryDrawLayerLooped(renderer, layers.PracticeLevelInfoExtreme, playbackFrame); break; }
			case Difficulty::ExExtreme: { TryDrawLayerLooped(renderer, layers.PracticeLevelInfoExExtreme, playbackFrame); break; }
			}

			TryDrawLayerLooped(renderer, layers.PracticeGaugeBase, playbackFrame);
			DrawHUDPracitceTime(renderer, hud.PlaybackTime);

			const auto progress = std::clamp(static_cast<f32>(hud.PlaybackTime / hud.Duration), 0.0f, 1.0f);
			const auto restartProgress = std::clamp(static_cast<f32>(hud.RestartTime / hud.Duration), 0.0f, 1.0f);

			TryDrawLayer(renderer, layers.PracticeGaugeTime, progress * 100.0f);
			TryDrawLayer(renderer, layers.PracticeGaugeBorderRestart, restartProgress * 100.0f);

			if (hud.DrawPracticeInfo && layers.PracticeInfo != nullptr)
			{
				// TODO: Make use of fade-in, -out and loop markers
				renderer.Aet().DrawLayer(*layers.PracticeInfo, 11.0f);
			}

			if (const auto* font = GetFont36(); font != nullptr && !hud.SongTitle.empty())
				renderer.Font().DrawBorder(*font, hud.SongTitle, TryGetTransform(layers.SongTitle, playbackFrame));
		}

		SyncHoldInfoMarkerData GetSyncHoldInfoMarkerData() const
		{
			return markers.SyncHoldInfo;
		}

		void DrawSyncHoldInfo(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const
		{
			std::array<ButtonType, 4> buttonIconTypes = { ButtonType::Count, ButtonType::Count, ButtonType::Count, ButtonType::Count };

			u32 syncCount = 0;
			if (data.TypeFlags & ButtonTypeFlags_Triangle) { buttonIconTypes[syncCount++] = ButtonType::Triangle; }
			if (data.TypeFlags & ButtonTypeFlags_Square) { buttonIconTypes[syncCount++] = ButtonType::Square; }
			if (data.TypeFlags & ButtonTypeFlags_Cross) { buttonIconTypes[syncCount++] = ButtonType::Cross; }
			if (data.TypeFlags & ButtonTypeFlags_Circle) { buttonIconTypes[syncCount++] = ButtonType::Circle; }

			if (syncCount < 1)
				return;

			auto getSyncInfoLayer = [this](const SyncHoldInfoData& data, u32 syncCount) -> auto&
			{
				switch (syncCount)
				{
				default:
				case 1: return layers.SyncInfoSingle;
				case 2: return data.TypeAdded ? layers.SyncInfoDoubleAdd : layers.SyncInfoDouble;
				case 3: return data.TypeAdded ? layers.SyncInfoTripleAdd : layers.SyncInfoTriple;
				case 4: return data.TypeAdded ? layers.SyncInfoQuadrupleAdd : layers.SyncInfoQuadruple;
				}
			};

			const auto* syncInfoLayer = getSyncInfoLayer(data, syncCount).get();
			if (syncInfoLayer == nullptr || syncInfoLayer->GetCompItem() == nullptr)
				return;

			if (data.Time.ToFrames() >= syncInfoLayer->EndFrame)
				return;

			char decimalString[16] = {};
			const auto decimalStringLen = sprintf_s(decimalString, "+%d", std::clamp(data.HoldScore, 0, 999999));

			const auto& compItem = *syncInfoLayer->GetCompItem();
			for (auto& layer : compItem.GetLayers())
			{
				const auto* videoItem = layer->GetVideoItem().get();
				for (size_t digitIndex = 0; digitIndex < videos.SyncInfoScoreDigitPlaceholders.size(); digitIndex++)
				{
					auto* digitVideo = videos.SyncInfoScoreDigitPlaceholders[digitIndex].get();
					if (videoItem == digitVideo)
					{
						const char decimalChar = (digitIndex <= decimalStringLen) ? decimalString[decimalStringLen - digitIndex] : '\0';
						if (decimalChar == '\0' || data.HideScore)
						{
							layer->RenderOverride.UseTexSpr = false;
						}
						else
						{
							const i8 digit = (decimalChar == '+') ? 10 : (decimalChar - '0');
							const auto texSpr = GetSyncInfoHoldNumerSprite(digit);
							layer->RenderOverride.UseTexSpr = true;
							layer->RenderOverride.Tex = texSpr.Tex;
							layer->RenderOverride.Spr = texSpr.Spr;
						}
					}
				}

				for (size_t buttonIconIndex = 0; buttonIconIndex < videos.SyncInfoButtonIconPlaceholders.size(); buttonIconIndex++)
				{
					auto* buttonVideo = videos.SyncInfoButtonIconPlaceholders[buttonIconIndex].get();
					if (videoItem == buttonVideo)
					{
						auto buttonType = buttonIconTypes[buttonIconIndex];
						if (buttonType < ButtonType::Count)
						{
							const auto texSpr = GetSyncInfoHoldButtonSprite(buttonType);
							layer->RenderOverride.UseTexSpr = true;
							layer->RenderOverride.Tex = texSpr.Tex;
							layer->RenderOverride.Spr = texSpr.Spr;
						}
						else
						{
							layer->RenderOverride.UseTexSpr = false;
						}
					}
				}
			}

			renderer.Aet().DrawLayer(*syncInfoLayer, data.Time.ToFrames());
		}

		void DrawSyncHoldInfoMax(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const
		{
			const auto* syncInfoMaxLayer = layers.SyncInfoMaxAdd.get();
			if (syncInfoMaxLayer == nullptr)
				return;

			if (data.Time.ToFrames() >= syncInfoMaxLayer->EndFrame)
				return;

			char decimalString[16] = {};
			const auto decimalStringLen = sprintf_s(decimalString, "+%d", std::clamp(data.HoldScore, 0, 999999));

			const auto& compItem = *syncInfoMaxLayer->GetCompItem();
			for (auto& layer : compItem.GetLayers())
			{
				const auto* videoItem = layer->GetVideoItem().get();
				for (size_t digitIndex = 0; digitIndex < videos.SyncInfoScoreDigitPlaceholders.size(); digitIndex++)
				{
					auto* digitVideo = videos.SyncInfoScoreDigitPlaceholders[digitIndex].get();
					if (videoItem == digitVideo)
					{
						const char decimalChar = (digitIndex <= decimalStringLen) ? decimalString[decimalStringLen - digitIndex] : '\0';
						if (decimalChar == '\0' || data.HideScore)
						{
							layer->RenderOverride.UseTexSpr = false;
						}
						else
						{
							const i8 digit = (decimalChar == '+') ? 10 : (decimalChar - '0');
							const auto texSpr = GetSyncInfoHoldNumerSprite(digit);
							layer->RenderOverride.UseTexSpr = true;
							layer->RenderOverride.Tex = texSpr.Tex;
							layer->RenderOverride.Spr = texSpr.Spr;
						}
					}
				}
			}

			renderer.Aet().DrawLayer(*syncInfoMaxLayer, data.Time.ToFrames());
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
					tempKeyFrameBackupStack.push(key.Curve);
					key.Curve = 0.0f;
				});
			};

			auto& layers = baseLayer.GetCompItem()->GetLayers();
			std::for_each(layers.begin(), layers.end(), [&](auto& layer)
			{
				push(layer->LayerVideo->Transform.Scale.X.Keys);
				push(layer->LayerVideo->Transform.Scale.Y.Keys);
			});
		}
		void PopRestoreScaleKeyFrames(const Aet::Layer& baseLayer) const
		{
			auto pop = [&](auto& keys)
			{
				std::for_each(keys.rbegin(), keys.rend(), [&](auto& key)
				{
					key.Curve = tempKeyFrameBackupStack.top();
					tempKeyFrameBackupStack.pop();
					key.Value = tempKeyFrameBackupStack.top();
					tempKeyFrameBackupStack.pop();
				});
			};

			auto& layers = baseLayer.GetCompItem()->GetLayers();
			std::for_each(layers.rbegin(), layers.rend(), [&](auto& layer)
			{
				pop(layer->LayerVideo->Transform.Scale.Y.Keys);
				pop(layer->LayerVideo->Transform.Scale.X.Keys);
			});
		}

		void PushHideHandLayers(const Aet::Layer& baseLayer) const
		{
			auto push = [&](Aet::Layer& layer)
			{
				if (layer.GetVideoItem().get() == videos.TargetHand.get())
				{
					tempLayerVisibleBackupStack.push(layer.Flags.VideoActive);
					layer.Flags.VideoActive = false;
				}
			};

			auto& layers = baseLayer.GetCompItem()->GetLayers();
			std::for_each(layers.begin(), layers.end(), [&](auto& layer)
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
				if (layer.GetVideoItem().get() == videos.TargetHand.get())
				{
					layer.Flags.VideoActive = tempLayerVisibleBackupStack.top();
					tempLayerVisibleBackupStack.pop();
				}
			};

			auto& layers = baseLayer.GetCompItem()->GetLayers();
			std::for_each(layers.rbegin(), layers.rend(), [&](auto& layer)
			{
				if (auto& comp = layer->GetCompItem(); comp != nullptr)
					std::for_each(comp->GetLayers().rbegin(), comp->GetLayers().rend(), [&](auto& l) { pop(*l); });
				else
					pop(*layer);
			});
		}

		void DrawTargetAppearEffect(Render::Renderer2D& renderer, const TargetAppearData& data) const
		{
			if (layers.TargetAppearEffect != nullptr)
				renderer.Aet().DrawLayer(*layers.TargetAppearEffect, data.Time.ToFrames(), Transform2D(data.Position));
		}

		void DrawTargetHitEffect(Render::Renderer2D& renderer, const TargetHitData& data) const
		{
			auto getLayer = [this](const TargetHitData& data) -> auto&
			{
				if (data.SlideL) return data.Chain ? layers.HitEffectsChainL[0] : layers.HitEffectsSlideL[data.CoolHit ? 0 : 1];
				if (data.SlideR) return data.Chain ? layers.HitEffectsChainR[0] : layers.HitEffectsSlideR[data.CoolHit ? 0 : 1];
				return layers.HitEffects[data.CoolHit ? 0 : data.FineHit ? 1 : data.SafeHit ? 2 : data.SadHit ? 3 : 4];
			};

			const auto* layer = getLayer(data).get();
			if (layer != nullptr)
				renderer.Aet().DrawLayer(*layer, data.Time.ToFrames(), Transform2D(data.Position));
		}

		void DrawTargetComboText(Render::Renderer2D& renderer, const TargetComboTextData& data) const
		{
			if (data.Evaluation == HitEvaluation::None)
				return;

			auto getValueTextLayer = [this](const TargetComboTextData& data) -> auto&
			{
				switch (data.Evaluation)
				{
				case HitEvaluation::Cool:
					return layers.ValueTextCool[(data.Precision == HitPrecision::Early) ? 0 : (data.Precision == HitPrecision::Late) ? 2 : 3];
				case HitEvaluation::Fine:
					return layers.ValueTextFine[(data.Precision == HitPrecision::Early) ? 0 : 2];
				case HitEvaluation::Safe:
					return layers.ValueTextSafe[0];
				case HitEvaluation::Sad:
					return layers.ValueTextSad[0];

				case HitEvaluation::WrongCool:
					return layers.ValueTextWrong[0];
				case HitEvaluation::WrongFine:
					return layers.ValueTextWrong[1];
				case HitEvaluation::WrongSafe:
					return layers.ValueTextWrong[2];
				case HitEvaluation::WrongSad:
					return layers.ValueTextWrong[3];

				case HitEvaluation::AlmostCool:
					return layers.ValueTextAlmost[0];
				case HitEvaluation::AlmostFine:
					return layers.ValueTextAlmost[1];
				case HitEvaluation::AlmostSafe:
					return layers.ValueTextAlmost[2];
				case HitEvaluation::AlmostSad:
					return layers.ValueTextAlmost[3];

				case HitEvaluation::Worst:
					return layers.ValueTextWorst[0];
				}

				return layers.ValueTextCool[0];
			};

			auto getComboLayer = [this](const TargetComboTextData& data) -> auto&
			{
				const auto digitPlace = GetComboDigitPlace(data.ComboCount);
				if (data.Evaluation == HitEvaluation::Cool)
					return (data.Precision == HitPrecision::Early ? layers.ComboCoolEarly : data.Precision == HitPrecision::Just ? layers.ComboCoolJust : layers.ComboCoolLate)[static_cast<u8>(digitPlace)];
				else
					return (data.Precision == HitPrecision::Early ? layers.ComboFineEarly : layers.ComboFineLate)[static_cast<u8>(digitPlace)];
			};

			if (data.ComboCount <= 1)
			{
				if (const auto* layer = getValueTextLayer(data).get(); layer != nullptr)
					renderer.Aet().DrawLayer(*layer, data.Time.ToFrames(), Transform2D(data.Position));
			}
			else if (const auto* layer = getComboLayer(data).get(); layer != nullptr)
			{
				char decimalString[8] = {};
				sprintf_s(decimalString, "%04d", std::clamp(data.ComboCount, 0, 9999));

				std::array<i8, 4> decimalDigits = { 0, 0, 0, 0 };
				for (i32 i = static_cast<i32>(decimalDigits.size()) - 1; i >= 0; i--)
					decimalDigits[i] = (decimalString[i] >= '0' && decimalString[i] <= '9') ? (decimalString[i] - '0') : 0;

				if (const auto& compItem = layer->GetCompItem(); compItem != nullptr)
				{
					for (auto& layer : compItem->GetLayers())
					{
						const auto* videoItem = layer->GetVideoItem().get();
						for (size_t digitIndex = 0; digitIndex < videos.ComboNumberDigitPlaceholders.size(); digitIndex++)
						{
							auto* digitVideo = videos.ComboNumberDigitPlaceholders[digitIndex].get();
							if (videoItem == digitVideo)
							{
								const auto texSpr = GetComboNumberSprite(decimalDigits[(decimalDigits.size() - 1) - digitIndex]);
								layer->RenderOverride.UseTexSpr = true;
								layer->RenderOverride.Tex = texSpr.Tex;
								layer->RenderOverride.Spr = texSpr.Spr;
							}
						}
					}
				}

				renderer.Aet().DrawLayer(*layer, data.Time.ToFrames(), Transform2D(data.Position));
			}
		}

		void DrawChainSlidePointText(Render::Renderer2D& renderer, const ChainSlidePointTextData& data) const
		{
			auto getLayer = [this](const ChainSlidePointTextData& data) -> auto&
			{
				return data.Max ? layers.MaxSlidePointOdd : layers.ChancePointOdd;
			};

			const auto* layer = getLayer(data).get();
			if (layer == nullptr)
				return;

			const auto inputFrame = data.Time.ToFrames();
			const auto inputFrameNumber = data.Max ? inputFrame : (inputFrame + 30.0f);

			const auto inputPositionTransform = Transform2D(data.Position + vec2(0.0f, +160.0f));

			if (const auto& compItem = layer->GetCompItem(); compItem != nullptr)
			{
				auto baseLayerTransform = Aet::Util::GetTransformAt(*layer->LayerVideo, inputFrame);
				Aet::Util::CombineTransforms(inputPositionTransform, baseLayerTransform);

				char decimalString[8] = {};
				sprintf_s(decimalString, "+%d", std::clamp(data.Points, 0, 99999));

				std::array<i8, 8> decimalDigits = {};
				for (i32 i = static_cast<i32>(decimalDigits.size()) - 1; i >= 0; i--)
					decimalDigits[i] = (decimalString[i] >= '0' && decimalString[i] <= '9') ? (decimalString[i] - '0') : (decimalString[i] == '+') ? 10 : -1;

				const auto nonNullDigitCount = std::count_if(decimalDigits.begin(), decimalDigits.end(), [](i8 d) { return (d != -1); });

				for (auto& layer : compItem->GetLayers())
				{
					const auto* videoItem = layer->GetVideoItem().get();
					for (i32 digitIndex = 0; digitIndex < static_cast<i32>(videos.ChancePointDigitPlaceholders.size()); digitIndex++)
					{
						auto* digitVideo = videos.ChancePointDigitPlaceholders[digitIndex].get();
						if (videoItem != digitVideo || digitIndex > nonNullDigitCount)
							continue;

						const auto sliderNumberIndex = decimalDigits[nonNullDigitCount - digitIndex];
						if (!InBounds(sliderNumberIndex, layers.MaxSlideNumbers))
							continue;

						const auto* maxSlideNumber = layers.MaxSlideNumbers[sliderNumberIndex].get();
						if (maxSlideNumber == nullptr)
							continue;

						auto numberTransform = Aet::Util::GetPositionTransformAt(*layer->LayerVideo, inputFrame);
						Aet::Util::CombineTransforms(baseLayerTransform, numberTransform);
						renderer.Aet().DrawLayer(*maxSlideNumber, inputFrameNumber, numberTransform);
					}
				}
			}

			renderer.Aet().DrawLayer(*layer, inputFrame, inputPositionTransform);
		}

		void DrawTarget(Render::Renderer2D& renderer, const TargetData& data) const
		{
			auto getLayerArray = [this](const TargetData& data) -> auto&
			{
				if (data.ChainHit)
					return !data.ChainStart ? layers.TargetsFragHit : layers.TargetsHit;

				if (data.Chain && !data.ChainStart)
					return data.Sync ? layers.TargetsFragSync : layers.TargetsFrag;

				if (data.Chance)
				{
					if (data.Sync)
						return data.HoldText ? layers.TargetsChanceSyncHold : layers.TargetsChanceSync;
					else
						return data.HoldText ? layers.TargetsChanceHold : layers.TargetsChance;
				}

				if (data.Sync)
					return data.HoldText ? layers.TargetsSyncHold : layers.TargetsSync;

				if (data.HoldText)
					return layers.TargetsHold;

				return layers.Targets;
			};

			if (data.Scale <= 0.0f)
				return;

			const auto* layer = getLayerArray(data)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			if (data.NoScale) PushSetScaleKeyFrames(*layer);
			if (data.NoHand) PushHideHandLayers(*layer);

			auto transform = Transform2D(data.Position);
			transform.Scale = vec2(data.Scale);
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
				if (data.Chain && !data.ChainStart)
					return data.Sync ? layers.ButtonsFragSync : layers.ButtonsFrag;

				return data.Sync ? layers.ButtonsSync : layers.Buttons;
			};

			if (data.Scale <= 0.0f)
				return;

			const auto* layer = getLayerArray(data)[static_cast<size_t>(data.Type)].get();
			if (layer == nullptr)
				return;

			auto transform = Transform2D(data.Position);
			transform.Scale = vec2(data.Scale);

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, (data.Progress * layerFrameScale), transform);
		}

		void DrawButtonShadow(Render::Renderer2D& renderer, const ButtonData& data) const
		{
			if (data.Scale <= 0.0f)
				return;

			const bool fragment = (data.Chain && !data.ChainStart);
			const auto* layer =
				(data.Shadow == ButtonShadowType::Black) ? (fragment ? layers.ButtonShadowsBlackFrag : layers.ButtonShadowsBlack)[static_cast<size_t>(data.Type)].get() :
				(data.Shadow == ButtonShadowType::White) ? (fragment ? layers.ButtonShadowsWhiteFrag : layers.ButtonShadowsWhite)[static_cast<size_t>(data.Type)].get() : nullptr;

			if (layer == nullptr)
				return;

			auto transform = Transform2D(data.Position);
			transform.Scale = vec2(data.Scale);

			constexpr auto layerFrameScale = 360.0f;
			renderer.Aet().DrawLayer(*layer, (data.Progress * layerFrameScale), transform);
		}

		void DrawButtonTrail(Render::Renderer2D& renderer, const ButtonTrailData& data) const
		{
			if (data.Opacity <= 0.0f)
				return;

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
				segmentPositions[i] = GetButtonPathSinePoint(glm::min(data.ProgressStart + (static_cast<f32>(i) * progressPerSegment), data.ProgressMax), data.Properties);

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
				const auto segmentColor = vec4(buttonColor, segmentAlpha * data.Opacity);

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
			if (data.SyncPairCount < 2 || data.Scale <= 0.0f || data.Opacity <= 0.0f)
				return;

			const auto syncLine = GetButtonSyncLineSprite();
			if (!syncLine)
				return;

			if (data.SyncPairCount == 2)
			{
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[0], data.ButtonPositions[1], data.Progress, data.Scale, data.Opacity);
			}
			else if (data.SyncPairCount == 3)
			{
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[0], data.ButtonPositions[1], data.Progress, data.Scale, data.Opacity);
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[1], data.ButtonPositions[2], data.Progress, data.Scale, data.Opacity);
				DrawSingleButtonSyncLine(renderer, syncLine, data.ButtonPositions[2], data.ButtonPositions[0], data.Progress, data.Scale, data.Opacity);
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

				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[0], sortedButtons[1], data.Progress, data.Scale, data.Opacity);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[1], sortedButtons[2], data.Progress, data.Scale, data.Opacity);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[2], sortedButtons[3], data.Progress, data.Scale, data.Opacity);
				DrawSingleButtonSyncLine(renderer, syncLine, sortedButtons[3], sortedButtons[0], data.Progress, data.Scale, data.Opacity);
			}
		}

		void DrawSingleButtonSyncLine(Render::Renderer2D& renderer, Render::TexSprView syncLine, vec2 start, vec2 end, f32 progress, f32 scale, f32 opacity) const
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

			commands[0].CornerColors[1].a = opacity;
			commands[0].CornerColors[3].a = opacity;
			commands[1].CornerColors[0].a = opacity;
			commands[1].CornerColors[2].a = opacity;

			commands[0].Position = start;
			commands[1].Position = (end + start) / 2.0f;

			commands[0].Origin = vec2(0.0f, spriteSizeHalf.y);
			commands[1].Origin = commands[0].Origin;

			auto vecToAngle = [](vec2 vec) { return glm::degrees(glm::atan(vec.y, vec.x)); };
			commands[0].Rotation = vecToAngle(end - start);
			commands[1].Rotation = commands[0].Rotation;

			commands[0].Scale = vec2((distance / spriteSize.x), -0.5f * scale);
			commands[1].Scale = commands[0].Scale;

			constexpr auto textureWidthsToScrollPerFullProgressCycle = 1.0f;
			const auto scroll = (progress * spriteSize.x * textureWidthsToScrollPerFullProgressCycle);

			commands[0].SourceRegion = vec4(scroll, 0.0f, spriteSizeHalf.x, spriteSize.y);
			commands[1].SourceRegion = vec4(scroll + spriteSizeHalf.x, 0.0f, spriteSizeHalf.x, spriteSize.y);

			for (const auto& command : commands)
				renderer.Draw(command);
		}

		BitmapFont* GetFont36() const
		{
			return (sprFont36 == nullptr || fontMap == nullptr) ? nullptr : IndexOrNull(font36Index, fontMap->Fonts);
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
			if (layer != nullptr) { renderer.Aet().DrawLayer(*layer, frame); }
		}

		void TryDrawLayerLooped(Render::Renderer2D& renderer, const std::shared_ptr<Aet::Layer>& layer, frame_t frame) const
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

		BitmapFont* GetFontPracticeNum() const
		{
			return (sprGame == nullptr || fontMap == nullptr) ? nullptr : IndexOrNull(fontPracticeNumIndex, fontMap->Fonts);
		}

		// TODO: Write util to easily and efficiently deal with aet number / character placeholders

		Render::TexSprView GetComboNumberSprite(i8 number) const
		{
			auto digitSprite = sprites.ComboNumbers[number];
			return (digitSprite == nullptr) ?
				Render::TexSprView { nullptr, nullptr } :
				Render::TexSprView { sprGameCommon->TexSet.Textures[digitSprite->TextureIndex].get(), digitSprite };
		}

		Render::TexSprView GetSyncInfoHoldNumerSprite(i8 number) const
		{
			auto digitSprite = sprites.SyncInfoHoldScoreNumbers[number];
			return (digitSprite == nullptr) ?
				Render::TexSprView { nullptr, nullptr } :
				Render::TexSprView { sprGameCommon->TexSet.Textures[digitSprite->TextureIndex].get(), digitSprite };
		}

		Render::TexSprView GetSyncInfoHoldButtonSprite(ButtonType type) const
		{
			auto buttonSprite = sprites.SyncInfoHoldButtonIcons[static_cast<u8>(type)];
			return (buttonSprite == nullptr) ?
				Render::TexSprView { nullptr, nullptr } :
				Render::TexSprView { sprGameCommon->TexSet.Textures[buttonSprite->TextureIndex].get(), buttonSprite };
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
				LifeGaugeInsurance,
				SongEnergyBase,
				SongEnergyNormal,
				SongIconLoop,
				LevelInfoEasy,
				LevelInfoNormal,
				LevelInfoHard,
				LevelInfoExtreme,
				LevelInfoExExtreme,
				SongTitle,
				SyncInfoSingle,
				SyncInfoDouble,
				SyncInfoDoubleAdd,
				SyncInfoTriple,
				SyncInfoTripleAdd,
				SyncInfoQuadruple,
				SyncInfoQuadrupleAdd,
				SyncInfoMaxAdd,
				TargetAppearEffect;

			std::array<std::shared_ptr<Aet::Layer>, 5>
				HitEffects;

			std::array<std::shared_ptr<Aet::Layer>, 2>
				HitEffectsSlideL,
				HitEffectsSlideR;

			std::array<std::shared_ptr<Aet::Layer>, 1>
				HitEffectsChainL,
				HitEffectsChainR;

			std::array<std::shared_ptr<Aet::Layer>, 4>
				ValueTextCool,
				ValueTextFine;

			std::array<std::shared_ptr<Aet::Layer>, 1>
				ValueTextSad,
				ValueTextSafe;

			std::array<std::shared_ptr<Aet::Layer>, 1>
				ValueTextWorst;

			std::array<std::shared_ptr<Aet::Layer>, 4>
				ValueTextWrong,
				ValueTextAlmost;

			std::array<std::shared_ptr<Aet::Layer>, EnumCount<ComboDigitPlace>()>
				ComboCoolEarly, ComboCoolJust, ComboCoolLate,
				ComboFineEarly, ComboFineLate;

			std::shared_ptr<Aet::Layer>
				MaxSlidePointOdd,
				MaxSlidePointEven,
				ChancePointOdd,
				ChancePointEven;

			std::array<std::shared_ptr<Aet::Layer>, 11>
				MaxSlideNumbers;

			std::array<std::shared_ptr<Aet::Layer>, EnumCount<ButtonType>()>
				Targets,
				TargetsHit,
				TargetsFrag,
				TargetsFragHit,
				TargetsSync,
				TargetsFragSync,
				TargetsHold,
				TargetsSyncHold,
				TargetsChance,
				TargetsChanceHold,
				TargetsChanceSync,
				TargetsChanceSyncHold,
				Buttons,
				ButtonsFrag,
				ButtonsSync,
				ButtonsFragSync,
				ButtonShadowsBlack,
				ButtonShadowsBlackFrag,
				ButtonShadowsWhite,
				ButtonShadowsWhiteFrag;

			std::shared_ptr<Aet::Layer>
				PracticeLevelInfoEasy,
				PracticeLevelInfoNormal,
				PracticeLevelInfoHard,
				PracticeLevelInfoExtreme,
				PracticeLevelInfoExExtreme,
				PracticeBackground,
				PracticeGaugeBase,
				PracticeGaugeBaseNumPointMin,
				PracticeGaugeBaseNumPointSec,
				PracticeGaugeBaseNumPointMS,
				PracticeGaugeTime,
				PracticeGaugeBorderPlay,
				PracticeGaugeBorderRestart,
				PracticeInfo;
		} layers = {};

		struct VideoCache
		{
			std::shared_ptr<Aet::Video>
				TargetHand;

			std::array<std::shared_ptr<Aet::Video>, 4>
				ComboNumberDigitPlaceholders;

			std::array<std::shared_ptr<Aet::Video>, 4>
				SyncInfoButtonIconPlaceholders;

			std::array<std::shared_ptr<Aet::Video>, 8>
				SyncInfoScoreDigitPlaceholders;

			std::array<std::shared_ptr<Aet::Video>, 7>
				ChancePointDigitPlaceholders;

			std::shared_ptr<Aet::Video>
				PracticeBackgroundCommon,
				PracticeColorBlack,
				PracticeLogoDummy,
				PracticeCoverDummy,
				PracticeCoverShadow,
				PracticeBackgroundDummy;

			std::shared_ptr<Aet::Layer>
				PracticeLogoDummyParentLayer,
				PracticeCoverDummyParentLayer,
				PracticeBackgroundDummyParentLayer;

		} videos = {};

		struct MarkerCache
		{
			SyncHoldInfoMarkerData SyncHoldInfo;
		} markers = {};

		struct SpriteCache
		{
			Spr* PracticeNumbers;

			Spr* ButtonTrail;
			Spr* ButtonSyncLine;

			std::array<Spr*, 10>
				ComboNumbers;

			std::array<Spr*, 11>
				SyncInfoHoldScoreNumbers;

			std::array<Spr*, EnumCount<ButtonType>()>
				SyncInfoHoldButtonIcons;

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

	void TargetRenderHelper::DrawBackground(Render::Renderer2D& renderer, const BackgroundData& background) const
	{
		impl->DrawBackground(renderer, background);
	}

	void TargetRenderHelper::DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const
	{
		impl->DrawHUD(renderer, hud);
	}

	TargetRenderHelper::SyncHoldInfoMarkerData TargetRenderHelper::GetSyncHoldInfoMarkerData() const
	{
		return impl->GetSyncHoldInfoMarkerData();
	}

	void TargetRenderHelper::DrawSyncHoldInfo(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const
	{
		impl->DrawSyncHoldInfo(renderer, data);
	}

	void TargetRenderHelper::DrawSyncHoldInfoMax(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const
	{
		impl->DrawSyncHoldInfoMax(renderer, data);
	}

	void TargetRenderHelper::DrawTargetAppearEffect(Render::Renderer2D& renderer, const TargetAppearData& data) const
	{
		impl->DrawTargetAppearEffect(renderer, data);
	}

	void TargetRenderHelper::DrawTargetHitEffect(Render::Renderer2D& renderer, const TargetHitData& data) const
	{
		impl->DrawTargetHitEffect(renderer, data);
	}

	void TargetRenderHelper::DrawTargetComboText(Render::Renderer2D& renderer, const TargetComboTextData& data) const
	{
		impl->DrawTargetComboText(renderer, data);
	}

	void TargetRenderHelper::DrawChainSlidePointText(Render::Renderer2D& renderer, const ChainSlidePointTextData& data) const
	{
		impl->DrawChainSlidePointText(renderer, data);
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

	const Graphics::BitmapFont* TargetRenderHelper::TryGetFont36() const
	{
		return impl->GetFont36();
	}
}

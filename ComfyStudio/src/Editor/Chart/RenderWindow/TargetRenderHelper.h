#pragma once
#include "Types.h"
#include "Render/Render.h"
#include "Time/TimeSpan.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/HitEvaluation.h"
#include "Graphics/Auth2D/Font/FontMap.h"

namespace Comfy::Studio::Editor
{
	enum class GameTheme : u8;

	// NOTE: Helper class to abstract away all of the aet and spr specific render interactions
	class TargetRenderHelper : NonCopyable
	{
	public:
		TargetRenderHelper();
		~TargetRenderHelper();

	public:
		void UpdateAsyncLoading(Render::Renderer2D& renderer);
		void SetGameTheme(GameTheme theme);
		void SetAetSprGetter(Render::Renderer2D& renderer);

		template <typename Func>
		void WithFont36(Func func) { if (const auto* font = TryGetFont36()) { func(*font); } }

		struct BackgroundData
		{
			bool DrawGrid;
			bool DrawDim;
			bool DrawCover;
			bool DrawLogo;
			bool DrawBackground;
			TimeSpan PlaybackTime;
			Render::TexSprView CoverSprite, LogoSprite, BackgroundSprite;
		};

		void DrawBackground(Render::Renderer2D& renderer, const BackgroundData& background) const;

		struct HUDData
		{
			std::string_view SongTitle;
			Difficulty Difficulty;
			TimeSpan PlaybackTime;
			TimeSpan RestartTime;
			TimeSpan Duration;
			bool DrawPracticeInfo;
		};

		void DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const;

		struct SyncHoldInfoMarkerData
		{
			TimeSpan LoopStart;
			TimeSpan LoopStartAdd;
			TimeSpan LoopEnd;
			TimeSpan ChargeEnd;
			TimeSpan MaxLoopStart;
			TimeSpan MaxChargeEnd;
			TimeSpan MaxLoopEnd;
		};

		SyncHoldInfoMarkerData GetSyncHoldInfoMarkerData() const;

		struct SyncHoldInfoData
		{
			TimeSpan Time;
			ButtonTypeFlags TypeFlags;
			bool TypeAdded;
			bool HideScore;
			i32 HoldScore;
		};

		void DrawSyncHoldInfo(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const;
		void DrawSyncHoldInfoMax(Render::Renderer2D& renderer, const SyncHoldInfoData& data) const;

		struct TargetAppearData
		{
			vec2 Position;
			TimeSpan Time;
		};

		void DrawTargetAppearEffect(Render::Renderer2D& renderer, const TargetAppearData& data) const;

		struct TargetHitData
		{
			vec2 Position;
			TimeSpan Time;
			bool SlideL, SlideR, Chain;
			bool CoolHit, FineHit, SafeHit, SadHit;
		};

		void DrawTargetHitEffect(Render::Renderer2D& renderer, const TargetHitData& data) const;

		struct TargetComboTextData
		{
			vec2 Position;
			TimeSpan Time;
			i32 ComboCount;
			HitEvaluation Evaluation;
			HitPrecision Precision;
		};

		void DrawTargetComboText(Render::Renderer2D& renderer, const TargetComboTextData& data) const;

		struct ChainSlidePointTextData
		{
			vec2 Position;
			TimeSpan Time;
			i32 Points;
			bool Max;
		};

		void DrawChainSlidePointText(Render::Renderer2D& renderer, const ChainSlidePointTextData& data) const;

		struct TargetData
		{
			ButtonType Type;
			bool NoHand;
			bool NoScale;
			bool Sync;
			bool HoldText;
			bool Chain;
			bool ChainStart;
			bool ChainHit;
			bool Chance;
			vec2 Position;
			f32 Progress;
			f32 Scale;
			f32 Opacity;
		};

		void DrawTarget(Render::Renderer2D& renderer, const TargetData& data) const;

		enum class ButtonShadowType : u8 { None, Black, White };

		struct ButtonData
		{
			ButtonType Type;
			ButtonShadowType Shadow;
			bool Sync;
			bool Chain;
			bool ChainStart;
			vec2 Position;
			f32 Progress;
			f32 Scale;
		};

		void DrawButton(Render::Renderer2D& renderer, const ButtonData& data) const;
		void DrawButtonShadow(Render::Renderer2D& renderer, const ButtonData& data) const;

		struct ButtonTrailData
		{
			ButtonType Type;
			bool Chance;
			TargetProperties Properties;
			f32 Progress;
			f32 ProgressStart;
			f32 ProgressEnd;
			f32 ProgressMax;
			f32 Opacity;
		};

		void DrawButtonTrail(Render::Renderer2D& renderer, const ButtonTrailData& data) const;

		struct ButtonSyncLineData
		{
			u32 SyncPairCount;
			f32 Progress;
			f32 Scale;
			f32 Opacity;
			std::array<vec2, 4> TargetPositions;
			std::array<vec2, 4> ButtonPositions;
		};

		void DrawButtonPairSyncLines(Render::Renderer2D& renderer, const ButtonSyncLineData& data) const;

	private:
		const Graphics::BitmapFont* TryGetFont36() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}

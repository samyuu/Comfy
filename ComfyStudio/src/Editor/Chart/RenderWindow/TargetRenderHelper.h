#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Render/Render.h"
#include "Time/TimeSpan.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/HitEvaluation.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Helper class to abstract away all of the aet and spr specific render interactions
	class TargetRenderHelper : NonCopyable
	{
	public:
		TargetRenderHelper();
		~TargetRenderHelper();

	public:
		void UpdateAsyncLoading(Render::Renderer2D& renderer);
		void SetAetSprGetter(Render::Renderer2D& renderer);

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
			bool IsPlayback;
			TimeSpan PlaybackTime;
			TimeSpan PlaybackTimeOnStart;
			TimeSpan Duration;
		};

		void DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const;

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
			bool Transparent;
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
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}

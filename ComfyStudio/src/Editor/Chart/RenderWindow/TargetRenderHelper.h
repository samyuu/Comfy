#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Render/Render.h"
#include "Time/TimeSpan.h"
#include "Editor/Chart/SortedTargetList.h"

namespace Comfy::Studio::Editor
{
	// NOTE: Helper class to abstract away all of the aet and spr specific render interactions
	class TargetRenderHelper : NonCopyable
	{
	public:
		TargetRenderHelper();
		~TargetRenderHelper();

	public:
		void UpdateAsyncLoading();
		void SetAetSprGetter(Render::Renderer2D& renderer);

		struct HUD
		{
			std::string_view SongName;

			bool IsPlayback;
			TimeSpan PlaybackTime;
			TimeSpan PlaybackTimeOnStart;
			TimeSpan Duration;
		};

		void DrawHUD(Render::Renderer2D& renderer, const HUD& hud) const;

		struct TargetDrawData
		{
			ButtonType Type;
			bool NoHand;
			bool NoScale;
			bool Sync;
			bool HoldText;
			bool Chain;
			bool ChainHit;
			vec2 Position;
			f32 Progress;
		};

		void DrawTarget(Render::Renderer2D& renderer, const TargetDrawData& data) const;

		enum class ButtonShadowType : u8 { None, Black, White };

		struct ButtonDrawData
		{
			ButtonType Type;
			ButtonShadowType Shadow;
			bool Sync;
			bool Chain;
			vec2 Position;
			f32 Progress;
		};

		void DrawButton(Render::Renderer2D& renderer, const ButtonDrawData& data) const;
		void DrawButtonShadow(Render::Renderer2D& renderer, const ButtonDrawData& data) const;

		struct ButtonTrailData
		{
			ButtonType Type;
			bool Chance;
			TargetProperties Properties;
			f32 Progress;
			f32 ProgressStart;
			f32 ProgressEnd;
		};

		void DrawButtonTrail(Render::Renderer2D& renderer, const ButtonTrailData& data) const;

		struct ButtonSyncLineData
		{
			u32 SyncPairCount;
			f32 Progress;
			std::array<vec2, 4> TargetPositions;
			std::array<vec2, 4> ButtonPositions;
		};

		void DrawButtonPairSyncLines(Render::Renderer2D& renderer, const ButtonSyncLineData& data) const;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}

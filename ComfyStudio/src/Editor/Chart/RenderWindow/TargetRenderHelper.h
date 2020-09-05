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

		struct HUDData
		{
			std::string_view SongName;

			bool IsPlayback;
			TimeSpan PlaybackTime;
			TimeSpan PlaybackTimeOnStart;
			TimeSpan Duration;
		};

		void DrawHUD(Render::Renderer2D& renderer, const HUDData& hud) const;

		struct TargetData
		{
			ButtonType Type;
			bool NoHand;
			bool Transparent;
			bool NoScale;
			bool Sync;
			bool HoldText;
			bool Chain;
			bool ChainHit;
			vec2 Position;
			f32 Progress;
		};

		void DrawTarget(Render::Renderer2D& renderer, const TargetData& data) const;

		enum class ButtonShadowType : u8 { None, Black, White };

		struct ButtonData
		{
			ButtonType Type;
			ButtonShadowType Shadow;
			bool Sync;
			bool Chain;
			vec2 Position;
			f32 Progress;
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

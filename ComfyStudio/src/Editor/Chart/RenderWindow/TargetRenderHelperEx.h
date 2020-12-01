#pragma once
#include "Types.h"
#include "TargetRenderHelper.h"

namespace Comfy::Studio::Editor
{
	using TargetRenderHelperExFlushFlags = u32;
	enum TargetRenderHelperExFlushFlagsEnum : TargetRenderHelperExFlushFlags
	{
		TargetRenderHelperExFlushFlags_None = 0,
		TargetRenderHelperExFlushFlags_NoTargets = 1 << 0,
		TargetRenderHelperExFlushFlags_NoButtons = 1 << 1,
	};

	// NOTE: Additional high level helper class to handle render order, trail specifics, etc.
	class TargetRenderHelperEx
	{
	public:
		TargetRenderHelper::TargetData& EmplaceTarget();
		TargetRenderHelper::TargetAppearData& EmplaceTargetAppear();
		TargetRenderHelper::TargetHitData& EmplaceTargetHit();
		TargetRenderHelper::ButtonData& EmplaceButton();
		TargetRenderHelper::ButtonTrailData& EmplaceButtonTrail();
		TargetRenderHelper::ButtonSyncLineData& EmplaceSyncLine();
		void Flush(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, TargetRenderHelperExFlushFlags flags = TargetRenderHelperExFlushFlags_None);

		void ConstructButtonTrail(TargetRenderHelper::ButtonTrailData& outData, ButtonType type, f32 progressClamped, f32 progressUnbound, const TargetProperties& properties, TimeSpan flyDuration, bool chance = false);

	private:
		struct TargetAndAppearPair
		{
			TargetRenderHelper::TargetData Target;
			std::optional<TargetRenderHelper::TargetAppearData> Appear;
		};

		std::vector<TargetAndAppearPair> targetsAndEffects;
		std::vector<TargetRenderHelper::TargetHitData> targetHits;
		std::vector<TargetRenderHelper::ButtonData> buttons;
		std::vector<TargetRenderHelper::ButtonTrailData> trails;
		std::vector<TargetRenderHelper::ButtonSyncLineData> syncLines;
	};
}

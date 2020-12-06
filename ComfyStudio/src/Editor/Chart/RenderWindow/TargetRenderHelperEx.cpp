#include "TargetRenderHelperEx.h"

namespace Comfy::Studio::Editor
{
	TargetRenderHelper::TargetData& TargetRenderHelperEx::EmplaceTarget()
	{
		return targetsAndEffects.emplace_back().Target;
	}

	TargetRenderHelper::TargetAppearData& TargetRenderHelperEx::EmplaceTargetAppear()
	{
		assert(!targetsAndEffects.empty());
		return targetsAndEffects.back().Appear.emplace();
	}

	TargetRenderHelper::TargetHitData& TargetRenderHelperEx::EmplaceTargetHit()
	{
		return targetHits.emplace_back();
	}

	TargetRenderHelper::ButtonData& TargetRenderHelperEx::EmplaceButton()
	{
		return buttons.emplace_back();
	}

	TargetRenderHelper::ButtonTrailData& TargetRenderHelperEx::EmplaceButtonTrail()
	{
		return trails.emplace_back();
	}

	TargetRenderHelper::ButtonSyncLineData& TargetRenderHelperEx::EmplaceSyncLine()
	{
		return syncLines.emplace_back();
	}

	void TargetRenderHelperEx::Flush(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, TargetRenderHelperExFlushFlags flags)
	{
		if (!(flags & TargetRenderHelperExFlushFlags_NoButtons))
		{
			for (const auto& data : trails)
				renderHelper.DrawButtonTrail(renderer, data);

			for (const auto& data : buttons)
				if (data.Shadow != TargetRenderHelper::ButtonShadowType::None) { renderHelper.DrawButtonShadow(renderer, data); }
		}

		if (!(flags & TargetRenderHelperExFlushFlags_NoTargets))
		{
			auto checkDrawTargetAndAppear = [&](const TargetAndAppearPair& data)
			{
				renderHelper.DrawTarget(renderer, data.Target);

				if (data.Appear.has_value())
					renderHelper.DrawTargetAppearEffect(renderer, data.Appear.value());
			};

			if (std::none_of(targetsAndEffects.begin(), targetsAndEffects.end(), [&](auto& data) { return data.Target.Chain; }))
			{
				for (const auto& data : targetsAndEffects)
					checkDrawTargetAndAppear(data);
			}
			else
			{
				// NOTE: Draw chain starts on top of child fragments to make sure the target hand is always fully visible
				enum DrawOrder : i8 { OrderChain, OrderChainStart, OrderChainHit, OrderChainStartHit, OrderCount };

				for (i8 orderPass = OrderChain; orderPass < OrderCount; orderPass++)
				{
					for (const auto& data : targetsAndEffects)
					{
						const auto targetOrder = data.Target.ChainStart ?
							data.Target.ChainHit ? OrderChainStartHit : OrderChainStart :
							data.Target.ChainHit ? OrderChainHit : OrderChain;

						if (targetOrder == orderPass)
							checkDrawTargetAndAppear(data);
					}
				}
			}

			for (const auto& data : targetHits)
				renderHelper.DrawTargetHitEffect(renderer, data);
		}

		if (!(flags & TargetRenderHelperExFlushFlags_NoButtons))
		{
			for (const auto& data : syncLines)
				renderHelper.DrawButtonPairSyncLines(renderer, data);

			for (const auto& data : buttons)
				renderHelper.DrawButton(renderer, data);
		}

		targetsAndEffects.clear();
		targetHits.clear();
		buttons.clear();
		trails.clear();
		syncLines.clear();
	}

	void TargetRenderHelperEx::ConstructButtonTrail(TargetRenderHelper::ButtonTrailData& outData, ButtonType type, f32 progressClamped, f32 progressUnbound, const TargetProperties& properties, TimeSpan flyDuration, bool chance)
	{
		// TODO: Fix up low BPM trail texture animation speed (?)
		constexpr f32 trailFactor = 2.55f;
		const f32 pixelLength = (properties.Distance / 1000.0f) * (240.0f / static_cast<f32>(flyDuration.TotalSeconds()) * trailFactor);
		const f32 normalizedLength = pixelLength / properties.Distance;

		outData.Type = type;
		outData.Chance = chance;
		outData.Properties = properties;
		outData.Progress = static_cast<f32>(flyDuration.TotalSeconds()) * progressUnbound;
		outData.ProgressStart = progressClamped;
		outData.ProgressEnd = glm::max(0.0f, (progressClamped - normalizedLength));
		outData.ProgressMax = 1.0f;
		outData.Opacity = 1.0f;
	}
}

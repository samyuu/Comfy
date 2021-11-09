#pragma once
#include "Tests/Game/Core/GameContext.h"

namespace Comfy::Sandbox::Tests::Game
{
	enum LoopState 
	{ 
		LoopState_In, 
		LoopState_Loop, 
		LoopState_Out, 
		LoopState_Count 
	};

	static constexpr std::array<std::string_view, LoopState_Count> LoopStartMarkerNames
	{
		"st_in",
		"st_lp",
		"st_out",
	};

	static constexpr std::array<std::string_view, LoopState_Count> LoopEndMarkerNames
	{
		"ed_in",
		"ed_lp",
		"ed_out",
	};

	frame_t LoopFrames(frame_t inputFrame, frame_t loopStart, frame_t loopEnd)
	{
		return std::fmod(inputFrame, (loopEnd - loopStart) - 1.0f) + loopStart;
	}

	TimeSpan LoopMarkers(const Aet::Layer& layer, TimeSpan inputTime, std::string_view startMarker, std::string_view endMarker)
	{
		const auto loopStart = layer.FindMarkerFrame(startMarker).value_or(0.0f);
		const auto loopEnd = Max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

		const auto inputFrame = inputTime.ToFrames();
		const auto loopedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

		return TimeSpan::FromFrames(loopedFrame);
	}

	TimeSpan ClampMarkers(const Aet::Layer& layer, TimeSpan inputTime, std::string_view startMarker, std::string_view endMarker)
	{
		const auto loopStart = layer.FindMarkerFrame(startMarker).value_or(0.0f);
		const auto loopEnd = Max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

		const auto inputFrame = Min(inputTime.ToFrames(), (loopEnd - loopStart) - 1.0f);
		const auto clampedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

		return TimeSpan::FromFrames(clampedFrame);
	}

	TimeSpan LoopMarkers(const Aet::Layer& layer, TimeSpan inputTime, LoopState loop)
	{
		return LoopMarkers(layer, inputTime, LoopStartMarkerNames[loop], LoopEndMarkerNames[loop]);
	}
}

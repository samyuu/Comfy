#pragma once
#include "GameContext.h"

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
		const auto loopEnd = std::max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

		const auto inputFrame = inputTime.ToFrames();
		const auto loopedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

		return TimeSpan::FromFrames(loopedFrame);
	}

	TimeSpan LoopMarkersOnce(const Aet::Layer& layer, TimeSpan inputTime, std::string_view startMarker, std::string_view endMarker)
	{
		return LoopMarkers(layer, inputTime, startMarker, endMarker);

		const auto loopStart = layer.FindMarkerFrame(startMarker).value_or(0.0f);
		const auto loopEnd = std::max(layer.FindMarkerFrame(endMarker).value_or(loopStart), loopStart);

		const auto inputFrame = std::min(inputTime.ToFrames(), (loopEnd - loopStart));
		const auto loopedFrame = LoopFrames(inputFrame, loopStart, loopEnd);

		return TimeSpan::FromFrames(loopedFrame);
	}

	TimeSpan LoopMarkers(const Aet::Layer& layer, TimeSpan inputTime, LoopState loop)
	{
		return LoopMarkers(layer, inputTime, LoopStartMarkerNames[loop], LoopEndMarkerNames[loop]);
	}

	class MenuTransition
	{
	public:
		struct DurationData
		{
			TimeSpan FadeIn;
			TimeSpan FadeLoop;
			TimeSpan FadeOut;
		};

	public:
		MenuTransition(DurationData duration) : duration(duration) {}
		~MenuTransition() = default;

	public:
		void Tick(Render::Renderer2D& renderer, TimeSpan elapsed, vec2 size)
		{
#if 0
			Gui::DEBUG_NOSAVE_WINDOW(__FUNCTION__"(): Debug", [&]
			{
				Gui::Text(__FUNCTION__"():");
				Gui::Text("CurrentState: FadeState::%s", std::array { "None", "In", "Loop", "Out", "Done" }[static_cast<size_t>(currentState)]);
				Gui::Text("ElapsedFadeTime: %s", elapsedFadeTime.ToString().c_str());
			});
#endif

			if (currentState == FadeState::None || currentState == FadeState::Done)
				return;

			auto checkAdvance = [&](auto stateToCheck, auto nextState, auto duration) { if (currentState == stateToCheck && elapsedFadeTime > duration) { AdvanceFadeState(nextState); } };
			checkAdvance(FadeState::In, FadeState::Loop, duration.FadeIn);
			checkAdvance(FadeState::Loop, FadeState::Out, duration.FadeLoop);
			checkAdvance(FadeState::Out, FadeState::Done, duration.FadeOut);

			const auto fadeAlpha = [&]
			{
				if (currentState == FadeState::In)
					return std::clamp(static_cast<f32>(elapsedFadeTime.TotalSeconds() / duration.FadeIn.TotalSeconds()), 0.0f, 1.0f);
				if (currentState == FadeState::Loop)
					return 1.0f;
				if (currentState == FadeState::Out)
					return std::clamp(1.0f - static_cast<f32>(elapsedFadeTime.TotalSeconds() / duration.FadeOut.TotalSeconds()), 0.0f, 1.0f);
				return 0.0f;
			}();

			elapsedFadeTime += elapsed;

			if (currentState != FadeState::Done)
				renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), size, vec4(fadeBaseColor, fadeAlpha)));
		}

		void StartFadeOnce(bool& transitionStarted) { if (!transitionStarted) { AdvanceFadeState(FadeState::In); transitionStarted = true; } }
		// void Reset() { AdvanceFadeState(FadeState::None); }
		bool ResetIfDone() { if (currentState != FadeState::None && currentState != FadeState::In) { /*AdvanceFadeState(FadeState::None);*/ return true; } return false; }

		bool IsFading() const { return currentState != FadeState::None; }
		//bool IsFadingIn() const { return currentState == FadeState::In; }
		//bool IsFadingOut() const { return (currentState == FadeState::Out) || (currentState == FadeState::Done); }

	private:
		enum class FadeState { None, In, Loop, Out, Done };

		void AdvanceFadeState(FadeState newState)
		{
			currentState = newState;
			elapsedFadeTime = TimeSpan::Zero();
		}

	private:
		vec3 fadeBaseColor = vec3(0.0f, 0.0f, 0.0f);
		DurationData duration;
		FadeState currentState = FadeState::None;
		TimeSpan elapsedFadeTime = TimeSpan::Zero();
	};
}

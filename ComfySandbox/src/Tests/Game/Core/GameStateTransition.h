#pragma once
#include "GameContext.h"

namespace Comfy::Sandbox::Tests::Game
{
	class GameStateTransition
	{
	public:
		GameStateTransition() = default;
		~GameStateTransition() = default;

	public:
		void Tick(Render::Renderer2D& renderer, TimeSpan elapsed, vec2 size)
		{
			if (currentState == FadeState::None)
				return;

			auto checkAdvance = [&](auto stateToCheck, auto nextState, auto duration) { if (currentState == stateToCheck && elapsedFadeTime > duration) { AdvanceFadeState(nextState); } };
			checkAdvance(FadeState::In, FadeState::Loop, fadeInDuration);
			checkAdvance(FadeState::Out, FadeState::None, fadeOutDuration);

			const auto fadeAlpha = [&]
			{
				if (currentState == FadeState::In)
					return std::clamp(static_cast<f32>(elapsedFadeTime.TotalSeconds() / fadeInDuration.TotalSeconds()), 0.0f, 1.0f);
				if (currentState == FadeState::Loop)
					return 1.0f;
				if (currentState == FadeState::Out)
					return std::clamp(1.0f - static_cast<f32>(elapsedFadeTime.TotalSeconds() / fadeOutDuration.TotalSeconds()), 0.0f, 1.0f);
				return 0.0f;
			}();

			elapsedFadeTime += elapsed;

			if (currentState != FadeState::None && fadeAlpha > 0.0f)
				renderer.Draw(Render::RenderCommand2D(vec2(0.0f, 0.0f), size, vec4(fadeBaseColor, fadeAlpha)));
		}

	public:
		void StartFadeIn(TimeSpan duration) { fadeInDuration = duration; AdvanceFadeState(FadeState::In); }
		void StartFadeOut(TimeSpan duration) { fadeOutDuration = duration; AdvanceFadeState(FadeState::Out); }

		bool HasFadedIn() const { return (currentState == FadeState::Loop); }

	private:
		enum class FadeState { None, In, Loop, Out };

		void AdvanceFadeState(FadeState newState)
		{
			currentState = newState;
			elapsedFadeTime = TimeSpan::Zero();
		}

	private:
		vec3 fadeBaseColor = vec3(0.0f, 0.0f, 0.0f);
		FadeState currentState = FadeState::None;
		TimeSpan elapsedFadeTime = TimeSpan::Zero();

		TimeSpan fadeInDuration, fadeOutDuration;
	};
}

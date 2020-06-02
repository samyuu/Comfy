#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "GameContext.h"
#include "GameState.h"
#include "GameStateTransition.h"

namespace Comfy::Sandbox::Tests::Game
{
	class GameStateManager : NonCopyable
	{
	public:
		GameStateManager(GameContext& context) : context(context)
		{
			for (size_t i = 0; i < static_cast<size_t>(GameStateType::Count); i++)
				allGameStates[i] = GameStateBase::CreateByType(static_cast<GameStateType>(i), context);

			assert(std::all_of(allGameStates.begin(), allGameStates.end(), [](const auto& gameState) { return (gameState != nullptr); }));

			ChangeGameState(startupGameStateType);
		}

	public:
		void Tick()
		{
			if (activeGameStateTypeAfterFadedEnded.has_value() && gameStateTransition.HasFadedIn())
			{
				if (fadeLoopElapsed >= fadeLoopDuration)
				{
					fadeLoopElapsed = TimeSpan::Zero();
					ChangeGameState(activeGameStateTypeAfterFadedEnded);
					activeGameStateTypeAfterFadedEnded = {};
					gameStateTransition.StartFadeOut(fadeOutDuration);
				}
				else
				{
					fadeLoopElapsed += context.Elapsed;
				}
			}

			if (!activeGameStateType.has_value())
			{
				context.Renderer.Font().DrawShadow(*context.Font36, "<NULL_GAME_STATE>", Transform2D(vec2(0.0f, 0.0f)));
				return;
			}

			GameStateBase* activeState = GetGameStateByType(activeGameStateType.value());
			if (activeState == nullptr)
			{
				context.Renderer.Font().DrawShadow(*context.Font36, "<INVALID_GAME_STATE>", Transform2D(vec2(0.0f, 0.0f)));
				return;
			}

			if (!activeGameStateTypeAfterFadedEnded.has_value())
				activeState->OnUpdateInput();

			activeState->OnDraw();

			if (const auto changeRequest = activeState->ChangeRequest; !activeGameStateTypeAfterFadedEnded.has_value() && changeRequest.has_value())
			{
				if (changeRequest->Fade)
				{
					gameStateTransition.StartFadeIn(fadeInDuration);
					activeGameStateTypeAfterFadedEnded = changeRequest->NewState;
				}
				else
				{
					ChangeGameState(changeRequest->NewState);
				}

				activeState->ChangeRequest = {};
			}

			gameStateTransition.Tick(context.Renderer, context.Elapsed, context.VirtualResolution);
		}

		void DebugGui()
		{
			Gui::TextUnformatted("ChangeGameState()");
			for (size_t i = 0; i < static_cast<size_t>(GameStateType::Count); i++)
			{
				if (Gui::Selectable(GameStateTypeNames[i].data()))
					ChangeGameState(static_cast<GameStateType>(i));
			}
		}

	private:
		GameStateBase* GetGameStateByType(std::optional<GameStateType> type) const
		{
			return (type.has_value()) ? GetGameStateByType(type.value()) : nullptr;
		}

		GameStateBase* GetGameStateByType(GameStateType type) const
		{
			const auto index = static_cast<size_t>(type);
			return (index < allGameStates.size()) ? allGameStates[index].get() : nullptr;
		}

		void ChangeGameState(std::optional<GameStateType> newStateType)
		{
			const auto previousStateType = activeGameStateType;
			activeGameStateType = newStateType;

			if (auto previousState = GetGameStateByType(previousStateType); previousState != nullptr)
				previousState->OnFocusLost(newStateType);

			if (auto newState = GetGameStateByType(newStateType); newState != nullptr)
				newState->OnFocusGained(previousStateType);
		}

	private:
		GameContext& context;

	private:
		const TimeSpan fadeInDuration = TimeSpan::FromSeconds(0.1f);
		const TimeSpan fadeLoopDuration = TimeSpan::FromSeconds(0.25f);
		const TimeSpan fadeOutDuration = TimeSpan::FromSeconds(0.1f);
		const std::optional<GameStateType> startupGameStateType = static_cast<GameStateType>(0);

	private:
		TimeSpan fadeLoopElapsed = TimeSpan::Zero();
		GameStateTransition gameStateTransition;

	private:
		std::optional<GameStateType> activeGameStateType = {};
		std::optional<GameStateType> activeGameStateTypeAfterFadedEnded = {};
		std::array<std::unique_ptr<GameStateBase>, static_cast<size_t>(GameStateType::Count)> allGameStates = {};
	};
}

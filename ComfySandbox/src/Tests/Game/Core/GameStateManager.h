#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "GameContext.h"
#include "GameState.h"
#include "GameStateTransition.h"
#include "Core/Logger.h"

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

			ChangeNotifyGameStateInternal(startupGameStateType);
		}

	public:
		void Tick()
		{
			if (!activeGameStateType.has_value())
			{
				DrawNullGameState();
				UpdateGameStateChangesRequest(nullptr);
			}
			else if (auto activeState = GetGameStateByType(activeGameStateType.value()); activeState == nullptr)
			{
				DrawInvalidGameState();
				UpdateGameStateChangesRequest(nullptr);
			}
			else
			{
				UpdateActiveGameState(*activeState);
				UpdateGameStateChangesRequest(activeState);
			}

			UpdateDrawGameStateTransition();
		}

		void DebugGui()
		{
			Gui::TextUnformatted("Game State Selection:");

			for (size_t i = 0; i <= static_cast<size_t>(GameStateType::Count); i++)
			{
				const auto type = static_cast<GameStateType>(i);

				if (Gui::Selectable(GetGameStateTypeName(type).data()))
					ChangeGameState(GameStateChangeRequest { type, true });
				if (Gui::IsItemClicked(1))
					ChangeGameState(GameStateChangeRequest { type, false });
			}
		}

	private:
		void DrawNullGameState()
		{
			context.Renderer.Font().DrawShadow(*context.Font36, "<NULL_GAME_STATE>", Transform2D(vec2(0.0f, 0.0f)));
		}

		void DrawInvalidGameState()
		{
			context.Renderer.Font().DrawShadow(*context.Font36, "<INVALID_GAME_STATE>", Transform2D(vec2(0.0f, 0.0f)));
		}

		void UpdateActiveGameState(GameStateBase& activeState)
		{
			// TODO: Remove this check (?) for OnDraw() at least
			if (!gameStateTransition.HasFadedIn())
			{
				if (!gameStateTypeAfterFadeEnded.has_value() && Gui::IsWindowFocused())
					activeState.OnUpdateInput();

				activeState.OnDraw();
			}
		}

		void UpdateGameStateChangesRequest(GameStateBase* activeState)
		{
			if (gameStateTypeAfterFadeEnded.has_value())
			{
				if (gameStateTransition.HasFadedIn())
				{
					if (fadeLoopElapsed >= fadeLoopDuration)
					{
						fadeLoopElapsed = TimeSpan::Zero();
						ChangeNotifyGameStateInternal(gameStateTypeAfterFadeEnded.value());
						gameStateTypeAfterFadeEnded = {};
						gameStateTransition.StartFadeOut(fadeOutDuration);
					}
					else
					{
						fadeLoopElapsed += context.Elapsed;
					}
				}
				else if (activeState != nullptr)
				{
					// NOTE: Explicitly ignore all requests if a request is already ongoing
					activeState->ChangeRequest = {};
				}
			}
			else if (activeState != nullptr)
			{
				if (const auto changeRequest = activeState->ChangeRequest; changeRequest.has_value())
				{
					ChangeGameState(changeRequest.value());
					activeState->ChangeRequest = {};
				}
			}
		}

		void UpdateDrawGameStateTransition()
		{
			gameStateTransition.Tick(context.Renderer, context.Elapsed, context.VirtualResolution);
		}

		GameStateBase* GetGameStateByType(std::optional<GameStateType> type) const
		{
			return (type.has_value()) ? GetGameStateByType(type.value()) : nullptr;
		}

		GameStateBase* GetGameStateByType(GameStateType type) const
		{
			const auto index = static_cast<size_t>(type);
			return (index < allGameStates.size()) ? allGameStates[index].get() : nullptr;
		}

		void ChangeGameState(GameStateChangeRequest change)
		{
			if (change.Fade)
			{
				gameStateTransition.StartFadeIn(fadeInDuration);
				gameStateTypeAfterFadeEnded = change.NewState;
			}
			else
			{
				ChangeNotifyGameStateInternal(change.NewState);
				gameStateTypeAfterFadeEnded = {};
				fadeLoopElapsed = TimeSpan::Zero();
				gameStateTransition.Reset();
			}
		}

		void ChangeNotifyGameStateInternal(GameStateType newStateType)
		{
			const auto previousStateType = activeGameStateType;
			activeGameStateType = newStateType;

			if (auto previousState = GetGameStateByType(previousStateType); previousState != nullptr)
				previousState->OnFocusLost(newStateType);

			if (auto newState = GetGameStateByType(newStateType); newState != nullptr)
				newState->OnFocusGained(previousStateType);

			Logger::LogLine(__FUNCTION__"(): [%s] -> [%s]", GetGameStateTypeName(previousStateType).data(), GetGameStateTypeName(newStateType).data());
		}

	private:
		GameContext& context;

	private:
#if 0 // DEBUG:
		const TimeSpan fadeInDuration = {}, fadeLoopDuration = {}, fadeOutDuration = {};
#else
		const TimeSpan fadeInDuration = TimeSpan::FromSeconds(0.12f);
		const TimeSpan fadeLoopDuration = TimeSpan::FromSeconds(0.24f);
		const TimeSpan fadeOutDuration = TimeSpan::FromSeconds(0.12f);
#endif

#if 1 // DEBUG:
		const GameStateType startupGameStateType = GameStateType::PS4GameMenu;
#else
		const GameStateType startupGameStateType = static_cast<GameStateType>(0);
#endif

	private:
		TimeSpan fadeLoopElapsed = TimeSpan::Zero();
		GameStateTransition gameStateTransition;

	private:
		std::optional<GameStateType> activeGameStateType = {};
		std::optional<GameStateType> gameStateTypeAfterFadeEnded = {};
		std::array<std::unique_ptr<GameStateBase>, static_cast<size_t>(GameStateType::Count)> allGameStates = {};
	};
}

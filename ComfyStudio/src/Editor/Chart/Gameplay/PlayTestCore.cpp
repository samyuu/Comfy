#include "PlayTestCore.h"
#include "PlayTestWindow.h"
#include "Core/ComfyStudioSettings.h"
#include "Time/Stopwatch.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		enum class SlidePositionType : u8
		{
			None,
			Left,
			Right,
			Any,
		};

		struct PlayTestInputBinding
		{
			ButtonTypeFlags ButtonTypes;
			SlidePositionType SlidePosition;
			std::variant<Input::KeyCode, Input::DS4Button> InputSource;
		};

		constexpr std::array PlayTestInputBindings =
		{
			PlayTestInputBinding { ButtonTypeFlags_Triangle, SlidePositionType::None, Input::KeyCode_W },
			PlayTestInputBinding { ButtonTypeFlags_Square, SlidePositionType::None, Input::KeyCode_A },
			PlayTestInputBinding { ButtonTypeFlags_Cross, SlidePositionType::None, Input::KeyCode_S },
			PlayTestInputBinding { ButtonTypeFlags_Circle, SlidePositionType::None, Input::KeyCode_D },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, SlidePositionType::Left, Input::KeyCode_Q },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, SlidePositionType::Left, Input::KeyCode_E },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, SlidePositionType::None, Input::KeyCode_I },
			PlayTestInputBinding { ButtonTypeFlags_Square, SlidePositionType::None, Input::KeyCode_J },
			PlayTestInputBinding { ButtonTypeFlags_Cross, SlidePositionType::None, Input::KeyCode_K },
			PlayTestInputBinding { ButtonTypeFlags_Circle, SlidePositionType::None, Input::KeyCode_L },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, SlidePositionType::Right, Input::KeyCode_U },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, SlidePositionType::Right, Input::KeyCode_O },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_1 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_2 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_3 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_4 },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_7 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_8 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_9 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::KeyCode_0 },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, SlidePositionType::None, Input::DS4Button::Triangle },
			PlayTestInputBinding { ButtonTypeFlags_Square, SlidePositionType::None, Input::DS4Button::Square },
			PlayTestInputBinding { ButtonTypeFlags_Cross, SlidePositionType::None, Input::DS4Button::Cross },
			PlayTestInputBinding { ButtonTypeFlags_Circle, SlidePositionType::None, Input::DS4Button::Circle },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, SlidePositionType::None, Input::DS4Button::DPad_Up },
			PlayTestInputBinding { ButtonTypeFlags_Square, SlidePositionType::None, Input::DS4Button::DPad_Left },
			PlayTestInputBinding { ButtonTypeFlags_Cross, SlidePositionType::None, Input::DS4Button::DPad_Down },
			PlayTestInputBinding { ButtonTypeFlags_Circle, SlidePositionType::None, Input::DS4Button::DPad_Right },

			PlayTestInputBinding { ButtonTypeFlags_SlideL, SlidePositionType::/*Any*/Left, Input::DS4Button::L1 },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, SlidePositionType::/*Any*/Right, Input::DS4Button::R1 },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, SlidePositionType::Left, Input::DS4Button::L_Stick_Left },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, SlidePositionType::Left, Input::DS4Button::L_Stick_Right },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, SlidePositionType::Right, Input::DS4Button::R_Stick_Left },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, SlidePositionType::Right, Input::DS4Button::R_Stick_Right },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::DS4Button::L_Trigger },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, SlidePositionType::None, Input::DS4Button::R_Trigger },
		};

		struct PlayTestTarget
		{
			ButtonType Type;
			SlidePositionType SlidePosition;
			TargetFlags Flags;
			TargetProperties Properties;

			TimeSpan RemainingTimeOnHit;
			HitEvaluation HitEvaluation;
			HitPrecision HitPrecision;

			bool HasBeenHit;
			bool HasBeenChainHit;
			bool HasAnyChainFragmentFailed;
			bool HasAnyChainHitAttemptBeenMade;
			bool HasAnyChainStartFragmentBeenHit;
			bool ThisFragmentCausedChainFailure;;
			bool HasTimedOut;
			bool WrongTypeOnTimeOut;
		};

		struct PlayTestSyncPair
		{
			std::array<PlayTestTarget, Rules::MaxSyncPairCount> Targets;
			u8 TargetCount;

			bool HasBeenAdded;
			bool NoLongerValid;

			TimeSpan TargetTime;
			TimeSpan ButtonTime;
			TimeSpan FlyDuration;

			vec2 PositionCenter;
		};

		struct SliderTouchPoint
		{
			SliderTouchPoint(f32 direction) : Direction(direction) {}

			f32 Direction;
			f32 NormalizedPosition = 0.5f;
			f32 Speed = 3.5f;
			Stopwatch LastSoundStopwatch = Stopwatch::StartNew();
			TimeSpan SoundInterval = TimeSpan::FromMilliseconds(50.0);
		};


		enum class PlayTestHoldEventType : u8
		{
			Start,
			Addition,
			Cancel,
			MaxOut,
			Count
		};

		struct PlayTestHoldEvent
		{
			PlayTestHoldEventType EventType;
			ButtonTypeFlags ButtonTypes;
			TimeSpan PlaybackTime;
			// TODO: Store index instead for safety
			const PlayTestSyncPair* SyncPair;
		};

		struct PlayTestHoldState
		{
			ButtonTypeFlags CurrentHoldTypes = ButtonTypeFlags_None;
			std::vector<PlayTestHoldEvent> EventHistory;
			// TODO: Store indices to avoid potential pointer invalidations, although that would make the code slightly less readable
			std::array<std::vector<const PlayTestInputBinding*>, EnumCount<ButtonType>()> PerTypeHeldDownBindings = {};

			void ClearAll()
			{
				CurrentHoldTypes = ButtonTypeFlags_None;
				EventHistory.clear();
				for (auto& bindings : PerTypeHeldDownBindings)
					bindings.clear();
			}

			void ClearAllHeldDownBindings()
			{
				for (auto& bindings : PerTypeHeldDownBindings)
					bindings.clear();
			}
		};

		constexpr ButtonTypeFlags GetSyncPairTypeFlags(const PlayTestSyncPair& syncPair)
		{
			ButtonTypeFlags holdFlags = ButtonTypeFlags_None;

			for (size_t i = 0; i < syncPair.TargetCount; i++)
				holdFlags |= ButtonTypeToButtonTypeFlags(syncPair.Targets[i].Type);

			return holdFlags;
		}

		constexpr ButtonTypeFlags GetSyncPairHoldTypeFlags(const PlayTestSyncPair& syncPair)
		{
			ButtonTypeFlags holdFlags = ButtonTypeFlags_None;

			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (syncPair.Targets[i].Flags.IsHold)
					holdFlags |= ButtonTypeToButtonTypeFlags(syncPair.Targets[i].Type);
			}

			return holdFlags;
		}

		constexpr bool AnyInSyncPairHasBeenHitByPlayer(const PlayTestSyncPair& syncPair)
		{
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (syncPair.Targets[i].HasBeenHit && !syncPair.Targets[i].HasTimedOut)
					return true;
			}

			return false;
		}

		constexpr bool AllInSyncPairHaveBeenHitByPlayer(const PlayTestSyncPair& syncPair)
		{
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (!syncPair.Targets[i].HasBeenHit || syncPair.Targets[i].HasTimedOut)
					return false;
			}

			return true;
		}

		constexpr bool AllInSyncPairHaveBeenHit(const PlayTestSyncPair& syncPair)
		{
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (!syncPair.Targets[i].HasBeenHit)
					return false;
			}

			return true;
		}

		constexpr bool IsCorrectTypeHitEvaluation(HitEvaluation hitEvaluation)
		{
			switch (hitEvaluation)
			{
			case HitEvaluation::Cool:
			case HitEvaluation::Fine:
			case HitEvaluation::Safe:
			case HitEvaluation::Sad:
				return true;
			default:
				return false;
			}
		}

		constexpr bool AllHoldsInSyncPairHaveBeenCorrectlyHitByPlayer(const PlayTestSyncPair& syncPair)
		{
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (syncPair.Targets[i].Flags.IsHold && syncPair.Targets[i].HasTimedOut && !IsCorrectTypeHitEvaluation(syncPair.Targets[i].HitEvaluation))
					return false;
			}

			return true;
		}

		template <typename Func>
		void ForEachFragmentInChain(std::vector<PlayTestSyncPair>& allPairs, PlayTestSyncPair& startFragmentPair, PlayTestTarget& startFragment, Func perFragmentFunc)
		{
			assert(startFragment.Flags.IsChain);
			const auto slideType = startFragment.Type;
			const auto firstFragmentIndex = std::distance(&allPairs[0], &startFragmentPair);

			for (size_t i = firstFragmentIndex; i < allPairs.size(); i++)
			{
				auto& pair = allPairs[i];
				for (size_t p = 0; p < pair.TargetCount; p++)
				{
					auto& potentialFragment = pair.Targets[p];

					if (!potentialFragment.Flags.IsChain || potentialFragment.Type != slideType)
						continue;

					perFragmentFunc(pair, potentialFragment);
					if (potentialFragment.Flags.IsChainEnd)
						return;
				}
			}
		}

		// BUG: Need to check window focus for DS4
		inline bool IsBindingPressed(const PlayTestInputBinding& binding)
		{
			if (auto key = std::get_if<Input::KeyCode>(&binding.InputSource))
				return Gui::IsKeyPressed(*key, false);
			else if (auto button = std::get_if<Input::DS4Button>(&binding.InputSource))
				return Input::DualShock4::IsTapped(*button);
			else
				assert(false);
			return false;
		}

		inline bool IsBindingDown(const PlayTestInputBinding& binding)
		{
			if (auto key = std::get_if<Input::KeyCode>(&binding.InputSource))
				return Gui::IsKeyDown(*key);
			else if (auto button = std::get_if<Input::DS4Button>(&binding.InputSource))
				return Input::DualShock4::IsDown(*button);
			else
				assert(false);
			return false;
		}

		inline bool IsBindingReleased(const PlayTestInputBinding& binding)
		{
			if (auto key = std::get_if<Input::KeyCode>(&binding.InputSource))
				return Gui::IsKeyReleased(*key);
			else if (auto button = std::get_if<Input::DS4Button>(&binding.InputSource))
				return Input::DualShock4::IsReleased(*button);
			else
				assert(false);
			return false;
		}

		inline void ChartToPlayTestTargets(Chart& chart, std::vector<PlayTestSyncPair>& outTargets)
		{
			outTargets.reserve(chart.Targets.size());

			for (size_t targetIndex = 0; targetIndex < chart.Targets.size();)
			{
				const auto& frontPairSourceTarget = chart.Targets[targetIndex];

				auto& newPair = outTargets.emplace_back();
				newPair.TargetCount = static_cast<u8>(std::min(static_cast<size_t>(frontPairSourceTarget.Flags.SyncPairCount), newPair.Targets.size()));
				for (size_t i = 0; i < newPair.TargetCount; i++)
				{
					const auto& sourceTarget = chart.Targets[targetIndex + i];

					auto& newTarget = newPair.Targets[i];
					newTarget.Type = sourceTarget.Type;
					newTarget.SlidePosition = SlidePositionType::None;
					newTarget.Flags = sourceTarget.Flags;
					newTarget.Properties = Rules::TryGetProperties(sourceTarget);
				}

				for (size_t i = 0; i < newPair.TargetCount; i++)
				{
					auto& thisSyncSlide = newPair.Targets[i];
					if (thisSyncSlide.Flags.IsSync && IsSlideButtonType(thisSyncSlide.Type))
					{
						auto& otherSyncSlide = newPair.Targets[(i == 0) ? 1 : 0];
						thisSyncSlide.SlidePosition = (thisSyncSlide.Properties.Position.x <= otherSyncSlide.Properties.Position.x) ? SlidePositionType::Left : SlidePositionType::Right;
					}
				}

				newPair.TargetTime = std::max(chart.TimelineMap.GetTimeAt(frontPairSourceTarget.Tick - BeatTick::FromBars(1)), TimeSpan::Zero());
				newPair.ButtonTime = chart.TimelineMap.GetTimeAt(frontPairSourceTarget.Tick);
				newPair.FlyDuration = (newPair.ButtonTime - newPair.TargetTime);

				for (size_t i = 0; i < newPair.TargetCount; i++)
					newPair.PositionCenter += newPair.Targets[i].Properties.Position;
				newPair.PositionCenter /= static_cast<f32>(newPair.TargetCount);

				assert(frontPairSourceTarget.Flags.SyncPairCount >= 1);
				targetIndex += frontPairSourceTarget.Flags.SyncPairCount;
			}
		}

		inline PlayTestSyncPair* FindBestSuitableUnhitSyncPairToEvaluateNext(std::vector<PlayTestSyncPair>& onScreenTargetPairs, TimeSpan playbackTime)
		{
			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid || AllInSyncPairHaveBeenHit(onScreenPair))
					continue;

				const auto remainingTime = (onScreenPair.ButtonTime - playbackTime);
				if (remainingTime <= HitThreshold::Sad && remainingTime >= -HitThreshold::Worst)
					return &onScreenPair;
			}

			return nullptr;
		}

		constexpr PlayTestTarget* FindBestSuitableUnhitTargetToEvaluateNext(PlayTestSyncPair& syncPair, ButtonType inputButtonType, SlidePositionType inputSlidePosition)
		{
			if (IsSlideButtonType(inputButtonType))
			{
				// NOTE: Prioritize slide position type
				for (size_t i = 0; i < syncPair.TargetCount; i++)
				{
					auto& target = syncPair.Targets[i];
					if (!target.HasBeenHit && !target.HasTimedOut && !target.Flags.IsChain && target.Type == inputButtonType && target.SlidePosition == inputSlidePosition)
						return &target;
				}

				for (size_t i = 0; i < syncPair.TargetCount; i++)
				{
					auto& target = syncPair.Targets[i];
					if (!target.HasBeenHit && !target.HasTimedOut && !target.Flags.IsChain && target.Type == inputButtonType)
						return &target;
				}
			}

			// NOTE: Prioritize button type
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				auto& target = syncPair.Targets[i];
				if (!target.HasBeenHit && !target.HasTimedOut && !target.Flags.IsChain && target.Type == inputButtonType)
					return &target;
			}

			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				auto& target = syncPair.Targets[i];
				if (!target.HasBeenHit && !target.HasTimedOut && !target.Flags.IsChain)
					return &target;
			}

			return nullptr;
		}
	}

	struct PlayTestCore::Impl
	{
	public:
		Impl(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext)
			: window(window), context(context), sharedContext(sharedContext)
		{
		}

	public:
		void UpdateTick()
		{
			UpdateUserInput();

			if (GetIsPlayback() && autoplayEnabled)
				UpdateAutoplayInput();

			CheckUpdateHoldStateMaxOut();

			DrawBackground();
			DrawUpdateOnScreenTargets();

			context.RenderHelperEx.Flush(*sharedContext.Renderer, *sharedContext.RenderHelper);

			DrawHUD();
			DrawScoreText();
			DrawPauseToggleFade();
			DrawOverlayText();
			DrawFadeInFadeOut();
		}

		void OverlayGui()
		{
			bool contextMenuOpen = false;
			Gui::WindowContextMenu("PlayTestWindowContextMenu", [&]
			{
				contextMenuOpen = true;

				if (Gui::MenuItem(GetIsPlayback() ? "Pause" : "Resume", Input::GetKeyCodeName(Input::KeyCode_Space)))
					TogglePause();

				if (Gui::MenuItem("Restart from Reset Point", Input::GetKeyCodeName(Input::KeyCode_Enter)))
					RestartFromRestartPoint();

				Gui::Separator();
				if (Gui::MenuItem("Move Reset Point Forward", "Tab"))
					MoveResetPointToPlaybackTime();

				if (Gui::MenuItem("Move Reset Point Backward", "Shift + Tab"))
					MoveResetPointBackward();

				Gui::Separator();

				Gui::MenuItem("Autoplay Enabled", Input::GetKeyCodeName(Input::KeyCode_F1), &autoplayEnabled);
				Gui::Separator();

				if (Gui::MenuItem("Return to Editor (Current)", Input::GetKeyCodeName(Input::KeyCode_Escape)))
					FadeOutThenExit(PlayTestExitType::ReturnCurrentTime);

				if (Gui::MenuItem("Return to Editor (Pre Playtest)", "Shift + Escape"))
					FadeOutThenExit(PlayTestExitType::ReturnPrePlayTestTime);

#if COMFY_DEBUG
				Gui::Separator();
				if (Gui::BeginMenu("Color Correction"))
				{
					Gui::Checkbox("Enabled", &context.RenderTarget->Param.PostProcessingEnabled);
					Gui::SliderFloat("Gamma", &context.RenderTarget->Param.PostProcessing.Gamma, 2.0f, 3.0f);
					Gui::SliderFloat("Contrast", &context.RenderTarget->Param.PostProcessing.Contrast, 0.35f, 0.80f);

					Gui::SliderFloat3("Coefficients R", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[0]), 0.0f, 1.25f);
					Gui::SliderFloat3("Coefficients G", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[1]), 0.0f, 1.25f);
					Gui::SliderFloat3("Coefficients B", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[2]), 0.0f, 1.25f);

					Gui::EndMenu();
				}
#endif
			});

			if (auto delta = Gui::GetIO().MouseDelta; delta.x != 0.0f || delta.y != 0.0f)
				mouseHide.LastMovementStopwatch.Restart();

			if (GlobalUserData.System.Video.PlaytestAutoHideCursor && !contextMenuOpen)
			{
				if (mouseHide.LastMovementStopwatch.GetElapsed() > mouseHide.AutoHideThreshold)
					Gui::SetMouseCursor(ImGuiMouseCursor_None);
			}
		}

		PlayTestExitType GetAndClearExitRequestThisFrame()
		{
			const auto exitRequest = exitRequestThisFrame;
			exitRequestThisFrame = PlayTestExitType::None;
			return exitRequest;
		}

		void Restart(TimeSpan startTime)
		{
			restartPoint = startTime;
			fadeInOut.InStopwatch = {};
			RestartFromRestartPoint();
		}

		bool GetAutoplayEnabled() const
		{
			return autoplayEnabled;
		}

		void SetAutoplayEnabled(bool value)
		{
			autoplayEnabled = value;

			// TODO: Think about handling this in a more optimal way
			if (!value)
				holdState.ClearAll();
		}

		bool GetIsPlayback() const
		{
			return sharedContext.SongVoice->GetIsPlaying();
		}

	private:
		void UpdateUserInput()
		{
			if (!Gui::IsWindowFocused())
				return;

			if (Gui::IsKeyPressed(Input::KeyCode_Escape, false))
				FadeOutThenExit(Gui::GetIO().KeyShift ? PlayTestExitType::ReturnPrePlayTestTime : PlayTestExitType::ReturnCurrentTime);

			if (Gui::IsKeyPressed(Input::KeyCode_F1, false))
				SetAutoplayEnabled(!GetAutoplayEnabled());

			if (Gui::IsKeyPressed(Input::KeyCode_Space, false) || Input::DualShock4::IsTapped(Input::DS4Button::Options))
				TogglePause();

			if (Gui::IsKeyPressed(Input::KeyCode_Enter, false) || Input::DualShock4::IsTapped(Input::DS4Button::R3))
				RestartFromRestartPoint();

			if (GetIsPlayback())
			{
				const bool shiftDown = Gui::GetIO().KeyShift;

				if ((shiftDown && Gui::IsKeyPressed(Input::KeyCode_Tab, false)) || Input::DualShock4::IsTapped(Input::DS4Button::L3))
					MoveResetPointBackward();

				if ((!shiftDown && Gui::IsKeyPressed(Input::KeyCode_Tab, false)) || Input::DualShock4::IsTapped(Input::DS4Button::Touch))
					MoveResetPointToPlaybackTime();

				if (!autoplayEnabled)
				{
					if (holdState.CurrentHoldTypes != ButtonTypeFlags_None)
					{
						for (size_t i = 0; i < EnumCount<ButtonType>(); i++)
						{
							auto& heldDownBindings = holdState.PerTypeHeldDownBindings[i];
							if (!heldDownBindings.empty())
							{
								heldDownBindings.erase(std::remove_if(heldDownBindings.begin(), heldDownBindings.end(), [](auto* b) { return !IsBindingDown(*b); }), heldDownBindings.end());

								if (heldDownBindings.empty())
									ProcessHoldStateCancelNow();
							}
						}
					}

					i32 chainSlideHoldCountL = 0, chainSlideHoldCountR = 0;

					for (const auto& binding : PlayTestInputBindings)
					{
						UpdateInputBindingButtonInputs(binding);

						if (IsBindingDown(binding))
						{
							chainSlideHoldCountL += ((binding.ButtonTypes & ButtonTypeFlags_SlideL) != 0);
							chainSlideHoldCountR += ((binding.ButtonTypes & ButtonTypeFlags_SlideR) != 0);
						}
					}

					if (chainSlideHoldCountL > 0)
						UpdateChainSlideDirection(ButtonType::SlideL, static_cast<f32>(chainSlideHoldCountL), sliderTouchPointL);
					else
						sliderTouchPointL.NormalizedPosition = 0.5f;

					if (chainSlideHoldCountR > 0)
						UpdateChainSlideDirection(ButtonType::SlideR, static_cast<f32>(chainSlideHoldCountR), sliderTouchPointR);
					else
						sliderTouchPointR.NormalizedPosition = 0.5f;
				}
			}
		}

		void DrawBackground()
		{
			const auto playbackTime = GetPlaybackTime();

			TargetRenderHelper::BackgroundData backgroundData = {};
			backgroundData.DrawGrid = true;
			backgroundData.DrawDim = true;
			backgroundData.DrawCover = true;
			backgroundData.DrawLogo = true;
			backgroundData.DrawBackground = true;
			backgroundData.PlaybackTime = playbackTime;
			backgroundData.CoverSprite = sharedContext.Chart->Properties.Image.Cover.GetTexSprView();
			backgroundData.LogoSprite = sharedContext.Chart->Properties.Image.Logo.GetTexSprView();
			backgroundData.BackgroundSprite = sharedContext.Chart->Properties.Image.Background.GetTexSprView();
			sharedContext.RenderHelper->DrawBackground(*sharedContext.Renderer, backgroundData);

			if (!holdState.EventHistory.empty())
			{
				const auto lastValidEvent = std::find_if(holdState.EventHistory.rbegin(), holdState.EventHistory.rend(), [&](const auto& e)
				{
					return (e.EventType == PlayTestHoldEventType::Start || e.EventType == PlayTestHoldEventType::Addition);
				});

				if (lastValidEvent != holdState.EventHistory.rend())
				{
					const auto& lastEvent = holdState.EventHistory.back();

					const auto syncHoldInfoMarkers = sharedContext.RenderHelper->GetSyncHoldInfoMarkerData();
					TargetRenderHelper::SyncHoldInfoData syncInfoData = {};

					const bool wasAddition = (lastValidEvent->EventType == PlayTestHoldEventType::Addition);

					const auto markerLoopStart = wasAddition ? syncHoldInfoMarkers.LoopStartAdd : syncHoldInfoMarkers.LoopStart;
					const auto markerLoopEnd = syncHoldInfoMarkers.LoopEnd;

					if (lastEvent.EventType == PlayTestHoldEventType::MaxOut)
					{
						const auto timeSinceMaxOut = (playbackTime - lastValidEvent->PlaybackTime - MaxTargetHoldDuration);
						syncInfoData.Time = (timeSinceMaxOut < syncHoldInfoMarkers.MaxLoopEnd) ?
							markerLoopStart + TimeSpan::FromSeconds(glm::mod((timeSinceMaxOut - markerLoopStart).TotalSeconds(), (markerLoopEnd - markerLoopStart).TotalSeconds())) :
							markerLoopEnd + (timeSinceMaxOut - syncHoldInfoMarkers.MaxLoopEnd);

						TargetRenderHelper::SyncHoldInfoData syncInfoMaxData = {};
						syncInfoMaxData.Time = timeSinceMaxOut;
						syncInfoMaxData.HoldScore = (ButtonTypeFlagsBitCount(lastValidEvent->ButtonTypes) * (6000 / 4));
						sharedContext.RenderHelper->DrawSyncHoldInfoMax(*sharedContext.Renderer, syncInfoMaxData);
					}
					else if (lastEvent.EventType == PlayTestHoldEventType::Cancel)
					{
						const auto timeSinceCancel = (playbackTime - lastEvent.PlaybackTime);
						syncInfoData.Time = markerLoopEnd + timeSinceCancel;
					}
					else
					{
						const auto timeSinceUpdate = (playbackTime - lastValidEvent->PlaybackTime);
						syncInfoData.Time = (timeSinceUpdate > markerLoopStart) ?
							markerLoopStart + TimeSpan::FromSeconds(glm::mod((timeSinceUpdate - markerLoopStart).TotalSeconds(), (markerLoopEnd - markerLoopStart).TotalSeconds())) :
							timeSinceUpdate;
					}

					syncInfoData.TypeFlags = lastValidEvent->ButtonTypes;
					syncInfoData.TypeAdded = wasAddition;

					const auto& events = holdState.EventHistory;
					const auto lastStartEventIndex = FindLastIndexOf(events, [](auto& e) { return e.EventType == PlayTestHoldEventType::Start; });

					syncInfoData.HoldScore = 0;

					for (size_t i = lastStartEventIndex; i < events.size(); i++)
					{
						if (events[i].EventType == PlayTestHoldEventType::MaxOut)
							break;

						// TODO: Or always use PlaybackTime..?
						auto getEventTime = [](const PlayTestHoldEvent& e) { return (e.SyncPair != nullptr) ? e.SyncPair->ButtonTime : e.PlaybackTime; };
						const auto endTime = (i + 1 == events.size()) ? playbackTime : getEventTime(events[i + 1]);

						const auto duration = (endTime - getEventTime(events[i]));
						const auto durationSixtyFPS = static_cast<i32>(glm::round(duration.ToFrames(60.0f)));

						constexpr i32 scorePerSixtyFPSFramePerTarget = 10;
						syncInfoData.HoldScore += (ButtonTypeFlagsBitCount(events[i].ButtonTypes) * scorePerSixtyFPSFramePerTarget * durationSixtyFPS);
					}

					sharedContext.RenderHelper->DrawSyncHoldInfo(*sharedContext.Renderer, syncInfoData);
				}
			}
		}

		void DrawUpdateOnScreenTargets()
		{
			const auto playbackTime = GetPlaybackTime();

			for (auto& availablePair : availableTargetPairs)
			{
				if (availablePair.HasBeenAdded)
					continue;

				if (availablePair.TargetTime <= playbackTime)
				{
					onScreenTargetPairs.push_back(availablePair);
					availablePair.HasBeenAdded = true;
				}
			}

			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid)
					continue;

				const auto elapsedTime = playbackTime - onScreenPair.TargetTime;
				const auto remainingTime = onScreenPair.ButtonTime - playbackTime;

				const auto progressUnbound = static_cast<f32>(ConvertRange(onScreenPair.TargetTime.TotalSeconds(), onScreenPair.ButtonTime.TotalSeconds(), 0.0, 1.0, playbackTime.TotalSeconds()));
				const auto progress = glm::clamp(progressUnbound, 0.0f, 1.0f);

				constexpr TimeSpan maxPostHitAnimationDuration = TimeSpan::FromSeconds(2.0);
				if (remainingTime < -maxPostHitAnimationDuration)
					onScreenPair.NoLongerValid = true;

				const auto hitMissProgress = std::clamp(static_cast<f32>(ConvertRange<f64>(0.0, -HitThreshold::Worst.TotalSeconds(), 1.0, 0.0, remainingTime.TotalSeconds())), 0.0f, 1.0f);

				for (size_t i = 0; i < onScreenPair.TargetCount; i++)
				{
					auto& onScreenTarget = onScreenPair.Targets[i];

					if (onScreenTarget.Flags.IsChain)
					{
						const auto chainSlot = (onScreenTarget.Type == ButtonType::SlideL) ? ChainSoundSlot::Left : ChainSoundSlot::Right;
						if (!onScreenTarget.HasBeenHit && onScreenTarget.HasBeenChainHit && !onScreenTarget.HasAnyChainFragmentFailed && remainingTime <= TimeSpan::Zero())
						{
							ForEachFragmentInChain(onScreenTargetPairs, onScreenPair, onScreenTarget, [&](PlayTestSyncPair& fragmentPair, PlayTestTarget& fragment)
							{
								fragment.HasAnyChainHitAttemptBeenMade = true;
							});

							onScreenTarget.HasBeenHit = true;
							onScreenTarget.RemainingTimeOnHit = TimeSpan::Zero();
							onScreenTarget.HitEvaluation = HitEvaluation::Cool;

							if (onScreenTarget.Flags.IsChainStart)
							{
								sharedContext.ButtonSoundController->PlayChainSoundStart(chainSlot);
								context.Score.ComboCount++;
								context.Score.ChainSlideScore = 0;

								ForEachFragmentInChain(onScreenTargetPairs, onScreenPair, onScreenTarget, [&](PlayTestSyncPair& fragmentPair, PlayTestTarget& fragment)
								{
									fragment.HasAnyChainStartFragmentBeenHit = true;
								});
							}
							else
							{
								if (onScreenTarget.Flags.IsChainEnd)
								{
									sharedContext.ButtonSoundController->FadeOutLastChainSound(chainSlot);
									sharedContext.ButtonSoundController->PlayChainSoundSuccess(chainSlot);
									context.Score.ChainSlideScore += 1000;
								}

								context.Score.ChainSlideScore += 10;
								lastSlideActionStopwatch.Restart();
							}
						}

						if (!onScreenTarget.HasBeenHit && !onScreenTarget.HasBeenChainHit && remainingTime < -HitThreshold::Worst)
						{
							if (!onScreenTarget.HasAnyChainFragmentFailed)
							{
								onScreenTarget.ThisFragmentCausedChainFailure = true;
								ForEachFragmentInChain(onScreenTargetPairs, onScreenPair, onScreenTarget, [&](PlayTestSyncPair& fragmentPair, PlayTestTarget& fragment)
								{
									fragment.HasAnyChainFragmentFailed = true;
								});

								if (onScreenTarget.HasAnyChainHitAttemptBeenMade)
								{
									sharedContext.ButtonSoundController->FadeOutLastChainSound(chainSlot);
									sharedContext.ButtonSoundController->PlayChainSoundFailure(chainSlot);
								}

								context.Score.ChainSlideScore = 0;
							}

							onScreenTarget.HasBeenHit = true;
							onScreenTarget.RemainingTimeOnHit = remainingTime;
							onScreenTarget.HasTimedOut = true;
							onScreenTarget.HitEvaluation = onScreenTarget.WrongTypeOnTimeOut ? HitEvaluation::WrongCool : HitEvaluation::Worst;
							onScreenTarget.HitPrecision = HitPrecision::Late;

							context.Score.ComboCount = 0;
						}
					}
					else if (!onScreenTarget.HasBeenHit && remainingTime < -HitThreshold::Worst)
					{
						onScreenTarget.HasBeenHit = true;
						onScreenTarget.RemainingTimeOnHit = remainingTime;
						onScreenTarget.HasTimedOut = true;
						onScreenTarget.HitEvaluation = onScreenTarget.WrongTypeOnTimeOut ? HitEvaluation::WrongCool : HitEvaluation::Worst;
						onScreenTarget.HitPrecision = HitPrecision::Late;

						if (onScreenTarget.WrongTypeOnTimeOut && IsSlideButtonType(onScreenTarget.Type))
							sharedContext.ButtonSoundController->PlaySlideSound();

						if (AllInSyncPairHaveBeenHit(onScreenPair))
							ProcessHoldStateForFullyHitSyncPair(onScreenPair);

						context.Score.ComboCount = 0;
					}

					auto properties = onScreenTarget.Properties;
					if (onScreenTarget.Flags.IsChain && !onScreenTarget.Flags.IsChainStart)
						properties.Position.x += Rules::ChainFragmentStartEndOffsetDistance * (onScreenTarget.Type == ButtonType::SlideL ? -1.0f : +1.0f);

					if (!onScreenTarget.HasBeenHit)
					{
						auto& targetData = context.RenderHelperEx.EmplaceTarget();
						targetData.Type = onScreenTarget.Type;
						targetData.Sync = onScreenTarget.Flags.IsSync;
						targetData.HoldText = onScreenTarget.Flags.IsHold;
						targetData.Chain = onScreenTarget.Flags.IsChain;
						targetData.ChainStart = onScreenTarget.Flags.IsChainStart;
						targetData.ChainHit = onScreenTarget.HasBeenChainHit;
						targetData.Chance = onScreenTarget.Flags.IsChance;
						targetData.Position = properties.Position;
						targetData.Progress = progressUnbound;
						targetData.Scale = hitMissProgress;

						auto& targetAppearData = context.RenderHelperEx.EmplaceTargetAppear();
						targetAppearData.Position = properties.Position;
						targetAppearData.Time = elapsedTime;
					}

					if (onScreenTarget.HasBeenHit && (!onScreenTarget.HasTimedOut || onScreenTarget.WrongTypeOnTimeOut))
					{
						const auto timeOnHit = onScreenPair.ButtonTime - onScreenTarget.RemainingTimeOnHit;
						const auto timeSinceHit = playbackTime - timeOnHit;

						auto& targetHitData = context.RenderHelperEx.EmplaceTargetHit();
						targetHitData.Position = properties.Position;
						targetHitData.Time = timeSinceHit;
						if (!onScreenTarget.WrongTypeOnTimeOut)
						{
							targetHitData.SlideL = (onScreenTarget.Type == ButtonType::SlideL);
							targetHitData.SlideR = (onScreenTarget.Type == ButtonType::SlideR);
							targetHitData.Chain = onScreenTarget.Flags.IsChain;
						}
						targetHitData.CoolHit = (onScreenTarget.HitEvaluation == HitEvaluation::Cool);
						targetHitData.FineHit = (onScreenTarget.HitEvaluation == HitEvaluation::Fine);
						targetHitData.SafeHit = (onScreenTarget.HitEvaluation == HitEvaluation::Safe);
						targetHitData.SadHit = (onScreenTarget.HitEvaluation == HitEvaluation::Sad);
					}

					if (!onScreenTarget.HasBeenHit)
					{
						auto& buttonData = context.RenderHelperEx.EmplaceButton();
						buttonData.Type = onScreenTarget.Type;
						buttonData.Sync = onScreenTarget.Flags.IsSync;
						buttonData.Chain = onScreenTarget.Flags.IsChain;
						buttonData.ChainStart = onScreenTarget.Flags.IsChainStart;
						buttonData.Shadow = TargetRenderHelper::ButtonShadowType::Black;
						buttonData.Position = GetButtonPathSinePoint(progressUnbound, properties);
						buttonData.Progress = progress;
						buttonData.Scale = hitMissProgress;
					}

					if (!onScreenTarget.Flags.IsSync && !onScreenTarget.HasBeenHit)
					{
						auto& trailData = context.RenderHelperEx.EmplaceButtonTrail();
						context.RenderHelperEx.ConstructButtonTrail(trailData, onScreenTarget.Type, progressUnbound, progressUnbound, properties, onScreenPair.FlyDuration);
						trailData.ProgressMax = std::numeric_limits<f32>::max();
						trailData.Opacity = hitMissProgress;
					}
				}

				if (onScreenPair.TargetCount > 1)
				{
					bool anyInSyncPairHit = false;
					for (size_t i = 0; i < onScreenPair.TargetCount; i++)
						anyInSyncPairHit |= onScreenPair.Targets[i].HasBeenHit;

					if (!anyInSyncPairHit)
					{
						auto& syncLineData = context.RenderHelperEx.EmplaceSyncLine();
						syncLineData.SyncPairCount = onScreenPair.TargetCount;
						syncLineData.Progress = progressUnbound;
						syncLineData.Scale = hitMissProgress;
						syncLineData.Opacity = hitMissProgress;
						for (size_t i = 0; i < onScreenPair.TargetCount; i++)
						{
							syncLineData.TargetPositions[i] = onScreenPair.Targets[i].Properties.Position;
							syncLineData.ButtonPositions[i] = GetButtonPathSinePoint(progressUnbound, onScreenPair.Targets[i].Properties);
						}
					}
				}
			}
		}

		void DrawHUD()
		{
			TargetRenderHelper::HUDData hudData = {};
			hudData.SongTitle = sharedContext.Chart->SongTitleOrDefault();
			hudData.Difficulty = sharedContext.Chart->Properties.Difficulty.Type;
			hudData.PlaybackTime = GetPlaybackTime();
			hudData.RestartTime = restartPoint;
			hudData.Duration = chartDuration;
			hudData.DrawPracticeInfo = true;
			sharedContext.RenderHelper->DrawHUD(*sharedContext.Renderer, hudData);
		}

		void DrawScoreText()
		{
			const auto playbackTime = GetPlaybackTime();

			bool comboTextDrawn = false;
			bool chainSlidePointTextDrawn = false;

			for (i32 i = static_cast<i32>(onScreenTargetPairs.size()) - 1; i >= 0; i--)
			{
				const auto& onScreenPair = onScreenTargetPairs[i];
				if (AllInSyncPairHaveBeenHit(onScreenPair))
				{
					const PlayTestTarget* latestHitTarget = &onScreenPair.Targets[0];
					for (size_t i = 1; i < onScreenPair.TargetCount; i++)
					{
						if (onScreenPair.Targets[i].RemainingTimeOnHit < latestHitTarget->RemainingTimeOnHit)
							latestHitTarget = &onScreenPair.Targets[i];
					}

					const auto timeOnHit = onScreenPair.ButtonTime - latestHitTarget->RemainingTimeOnHit;
					const auto timeSinceHit = playbackTime - timeOnHit;

					if (latestHitTarget->Flags.IsChain && (!latestHitTarget->Flags.IsChainStart && !latestHitTarget->ThisFragmentCausedChainFailure))
					{
						if (latestHitTarget->HasBeenChainHit && !chainSlidePointTextDrawn && context.Score.ChainSlideScore > 0)
						{
							chainSlidePointTextDrawn = true;
							TargetRenderHelper::ChainSlidePointTextData slidePointText = {};
							slidePointText.Position = onScreenPair.PositionCenter;
							slidePointText.Time = timeSinceHit;
							slidePointText.Points = context.Score.ChainSlideScore;
							slidePointText.Max = latestHitTarget->Flags.IsChainEnd;
							sharedContext.RenderHelper->DrawChainSlidePointText(*sharedContext.Renderer, slidePointText);
						}

						continue;
					}

					if (!comboTextDrawn)
					{
						comboTextDrawn = true;
						TargetRenderHelper::TargetComboTextData comboTextData = {};
						comboTextData.Position = glm::clamp(onScreenPair.PositionCenter, vec2(0.0f, Rules::ComboTextMinHeight), Rules::PlacementAreaSize);
						comboTextData.Time = timeSinceHit;
						comboTextData.ComboCount = context.Score.ComboCount;
						comboTextData.Evaluation = latestHitTarget->HitEvaluation;
						comboTextData.Precision = latestHitTarget->HitPrecision;

						sharedContext.RenderHelper->DrawTargetComboText(*sharedContext.Renderer, comboTextData);
					}

					if (comboTextDrawn && chainSlidePointTextDrawn)
						return;
				}
			}
		}

		void DrawBlackFullscreenQuad(f32 opactiy, f32 min = 0.0f, f32 max = 1.0f)
		{
			sharedContext.Renderer->Draw(Render::RenderCommand2D(vec2(0.0f), Rules::PlacementAreaSize, vec4(0.0f, 0.0f, 0.0f, std::clamp(opactiy, min, max))));
		}

		void DrawPauseToggleFade()
		{
			pauseFade.PlaybackLastFrame = pauseFade.PlaybackThisFrame;
			pauseFade.PlaybackThisFrame = GetIsPlayback();

			if (pauseFade.PlaybackLastFrame && !pauseFade.PlaybackThisFrame)
				pauseFade.InStopwatch.Restart();

			if (!pauseFade.PlaybackLastFrame && pauseFade.PlaybackThisFrame)
				pauseFade.OutStopwatch.Restart();

			if (pauseFade.InStopwatch.IsRunning())
			{
				const auto elapsed = pauseFade.InStopwatch.GetElapsed();
				DrawBlackFullscreenQuad(static_cast<f32>(ConvertRange<f64>(0.0, pauseFade.Duration.TotalSeconds(), 0.0, pauseFade.Opacity, elapsed.TotalSeconds())), 0.0f, pauseFade.Opacity);

				if (elapsed > pauseFade.Duration)
					pauseFade.InStopwatch.Stop();
			}
			else if (!pauseFade.PlaybackThisFrame)
			{
				DrawBlackFullscreenQuad(pauseFade.Opacity);
			}

			if (pauseFade.OutStopwatch.IsRunning())
			{
				const auto elapsed = pauseFade.OutStopwatch.GetElapsed();
				DrawBlackFullscreenQuad(static_cast<f32>(ConvertRange<f64>(0.0, pauseFade.Duration.TotalSeconds(), pauseFade.Opacity, 0.0, elapsed.TotalSeconds())), 0.0f, pauseFade.Opacity);

				if (elapsed > pauseFade.Duration)
					pauseFade.OutStopwatch.Stop();
			}
		}

		void DrawOverlayText()
		{
			char textBuffer[64] = {};
			if (!GetIsPlayback())
				strcat_s(textBuffer, "PAUSED");

			if (autoplayEnabled)
			{
				if (textBuffer[0] != '\0')
					strcat_s(textBuffer, "\n");
				strcat_s(textBuffer, "AUTOPLAY");
			}

			if (textBuffer[0] != '\0')
			{
				sharedContext.RenderHelper->WithFont36([&](auto& font)
				{
					constexpr auto rightAllignedTextPosition = vec2(16.0f, 112.0f + 10.0f);
					const auto textWidth = 180.0f;
					const auto textPosition = vec2(Rules::PlacementAreaSize.x - (textWidth + rightAllignedTextPosition.x), rightAllignedTextPosition.y);
					sharedContext.Renderer->Font().DrawBorder(font, textBuffer, Graphics::Transform2D(textPosition));
				});
			}

#if COMFY_DEBUG && 0 // DEBUG: Hold state debug view
			sharedContext.RenderHelper->WithFont36([&](auto& font)
			{
				static constexpr std::array buttonTypeNames = { "Triangle", "Square", "Cross", "Circle", "Slide L", "Slide R" };
				static constexpr std::array buttonTypeSymbols = { u8"△", u8"□", u8"×", u8"〇", u8"←", u8"→" };

				char b[512];
				auto buttonTypeFlagsToString = [&b](ButtonTypeFlags flags)
				{
					static std::string buffer; buffer.clear();
					buffer += "(";
					for (u32 i = 0; i < static_cast<u32>(EnumCount<ButtonType>()); i++)
					{
						if (flags & (1 << i))
							buffer += std::string_view(b, sprintf_s(b, "%s|", buttonTypeNames[i]));
					}
					buffer.erase(buffer.size() - std::string_view("|").size());
					buffer += ")";
					return buffer;
				};

				static std::string buffer; buffer.clear();
				buffer += "CurrentHoldTypes: ";
				if (holdState.CurrentHoldTypes == ButtonTypeFlags_None)
					buffer += "(None)";
				else
					buffer += buttonTypeFlagsToString(holdState.CurrentHoldTypes);
				buffer += '\n';

				buffer += "--------------------------------------\n";
				for (u32 i = 0; i < static_cast<u32>(EnumCount<ButtonType>()); i++)
					buffer += std::string_view(b, sprintf_s(b, "%s HeldDownBindings: %zu\n", buttonTypeSymbols[i], holdState.PerTypeHeldDownBindings[i].size()));
				buffer += "--------------------------------------\n";

				constexpr size_t recentHistoryDisplayCount = 6;
				for (size_t i = 0; i < std::min(holdState.EventHistory.size(), recentHistoryDisplayCount); i++)
				{
					const auto& holdEvent = holdState.EventHistory[holdState.EventHistory.size() - (i + 1)];
					const auto holdEventTimeStr = holdEvent.PlaybackTime.FormatTime();
					if (holdEvent.EventType == PlayTestHoldEventType::Start)
						buffer += std::string_view(b, sprintf_s(b, "%s - Start  - %s\n", holdEventTimeStr.data(), buttonTypeFlagsToString(holdEvent.ButtonTypes).c_str()));
					else if (holdEvent.EventType == PlayTestHoldEventType::Addition)
						buffer += std::string_view(b, sprintf_s(b, "%s - Add    - %s\n", holdEventTimeStr.data(), buttonTypeFlagsToString(holdEvent.ButtonTypes).c_str()));
					else if (holdEvent.EventType == PlayTestHoldEventType::Cancel)
						buffer += std::string_view(b, sprintf_s(b, "%s - Cancel - ...\n", holdEventTimeStr.data()));
					else if (holdEvent.EventType == PlayTestHoldEventType::MaxOut)
						buffer += std::string_view(b, sprintf_s(b, "%s - MaxOut - ...\n", holdEventTimeStr.data()));
				}

				sharedContext.Renderer->Font().DrawBorder(font, buffer, Graphics::Transform2D(vec2(0.0f, 0.0f), vec2(16.0f, 128.0f), 0.0f, vec2(1.0f / 1.5f), 1.0f));
			});
#endif
		}

		void DrawFadeInFadeOut()
		{
			if (fadeInOut.InStopwatch.IsRunning())
			{
				const auto elapsed = fadeInOut.InStopwatch.GetElapsed();
				DrawBlackFullscreenQuad(static_cast<f32>(ConvertRange<f64>(0.0, fadeInOut.InDuration.TotalSeconds(), 1.0, 0.0, elapsed.TotalSeconds())));

				if (elapsed > fadeInOut.InDuration)
					fadeInOut.InStopwatch.Stop();
			}

			if (fadeInOut.OutExitStopwatch.IsRunning())
			{
				const auto elapsed = fadeInOut.OutExitStopwatch.GetElapsed();
				DrawBlackFullscreenQuad(static_cast<f32>(ConvertRange<f64>(0.0, fadeInOut.OutDuration.TotalSeconds(), 0.0, 1.0, elapsed.TotalSeconds())));

				if (elapsed > fadeInOut.OutDuration)
				{
					exitRequestThisFrame = fadeInOut.OutExitType;
					fadeInOut.OutExitStopwatch.Stop();
				}
			}
		}

	private:
		void PlayOneShotSoundEffect(std::string_view name, f32 volume = 1.0f)
		{
			auto& audioEngine = Audio::AudioEngine::GetInstance();
			audioEngine.EnsureStreamRunning();
			audioEngine.PlayOneShotSound(sharedContext.SoundEffectManager->Find(name), name, volume);
		}

		void TogglePause()
		{
			const bool isPlaying = GetIsPlayback();
			PlayOneShotSoundEffect(isPlaying ? "se_ft_sys_dialog_open" : "se_ft_sys_dialog_close");
			sharedContext.SongVoice->SetIsPlaying(!isPlaying);

			if (isPlaying)
			{
				for (size_t i = 0; i < EnumCount<ChainSoundSlot>(); i++)
					sharedContext.ButtonSoundController->FadeOutLastChainSound(static_cast<ChainSoundSlot>(i));
			}
		}

		void FadeOutThenExit(PlayTestExitType exitType)
		{
			assert(exitType != PlayTestExitType::None);

			if (!fadeInOut.OutExitStopwatch.IsRunning())
			{
				fadeInOut.OutExitStopwatch.Restart();
				fadeInOut.OutExitType = exitType;
			}
		}

		void RestartFromRestartPoint()
		{
			if (fadeInOut.InStopwatch.IsRunning())
				return;

			fadeInOut.InStopwatch.Restart();
			PlayOneShotSoundEffect("se_ft_practice_restart_01");
			RestartInitializeChart(restartPoint);
		}

		void MoveResetPointBackward()
		{
			PlayOneShotSoundEffect("se_ft_practice_cursor_01");
			restartPoint = std::clamp(TimeSpan::FloorToSeconds(restartPoint - TimeSpan::FromSeconds(10.0)), TimeSpan::Zero(), chartDuration);
		}

		void MoveResetPointToPlaybackTime()
		{
			PlayOneShotSoundEffect("se_ft_practice_cursor_01");
			restartPoint = std::clamp(TimeSpan::FloorToSeconds(GetPlaybackTime()), TimeSpan::Zero(), chartDuration);
		}

		void RestartInitializeChart(TimeSpan startTime, bool resetScore = true)
		{
			if (GetIsPlayback())
			{
				for (size_t i = 0; i < EnumCount<ChainSoundSlot>(); i++)
					sharedContext.ButtonSoundController->FadeOutLastChainSound(static_cast<ChainSoundSlot>(i));
			}

			holdState.ClearAll();

			SetPlaybackTime(startTime);
			sharedContext.SongVoice->SetIsPlaying(true);

			chartDuration = sharedContext.Chart->DurationOrDefault();
			availableTargetPairs.clear();
			ChartToPlayTestTargets(*sharedContext.Chart, availableTargetPairs);
			onScreenTargetPairs.clear();
			onScreenTargetPairs.reserve(availableTargetPairs.size());

			for (auto& pair : availableTargetPairs)
			{
				if (pair.TargetTime < startTime)
				{
					pair.NoLongerValid = true;

					for (size_t i = 0; i < pair.TargetCount; i++)
					{
						// NOTE: Ensure all chains always have a valid start fragment
						if (pair.Targets[i].Flags.IsChain)
							ForEachFragmentInChain(availableTargetPairs, pair, pair.Targets[i], [&](auto& chainPair, auto& fragment) { chainPair.NoLongerValid = true; });
					}
				}
			}

			if (resetScore)
				context.Score = {};
		}

		void UpdateAutoplayInput()
		{
			const auto playbackTime = GetPlaybackTime();
			const auto deltaPerfectHitThreshold = TimeSpan::FromSeconds(Gui::GetIO().DeltaTime * 0.5f);

			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid || AllInSyncPairHaveBeenHit(onScreenPair))
					continue;

				const auto remainingTime = (onScreenPair.ButtonTime - playbackTime);
				if (remainingTime <= deltaPerfectHitThreshold)
				{
					for (size_t i = 0; i < onScreenPair.TargetCount; i++)
					{
						auto& target = onScreenPair.Targets[i];
						if (target.HasBeenHit || target.Flags.IsChain)
							continue;

						if (IsSlideButtonType(target.Type))
							sharedContext.ButtonSoundController->PlaySlideSound();
						else
							sharedContext.ButtonSoundController->PlayButtonSound();

						target.RemainingTimeOnHit = TimeSpan::Zero();
						target.HasBeenHit = true;
						target.HitEvaluation = HitEvaluation::Cool;
						target.HitPrecision = HitPrecision::Just;

						if (AllInSyncPairHaveBeenHit(onScreenPair))
						{
							context.Score.ComboCount++;

							ProcessHoldStateForFullyHitSyncPair(onScreenPair);
						}

					}
				}
			}

			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid || AllInSyncPairHaveBeenHit(onScreenPair))
					continue;

				for (size_t i = 0; i < onScreenPair.TargetCount; i++)
				{
					auto& target = onScreenPair.Targets[i];
					if (!target.Flags.IsChain || target.HasBeenChainHit)
						continue;

					if (!target.HasAnyChainStartFragmentBeenHit)
					{
						const auto remainingTime = (onScreenPair.ButtonTime - playbackTime);
						if (remainingTime <= deltaPerfectHitThreshold)
							target.HasBeenChainHit = true;
					}
					else
					{
						const auto progress = static_cast<f32>(ConvertRange(onScreenPair.TargetTime.TotalSeconds(), onScreenPair.ButtonTime.TotalSeconds(), 0.0, 1.0, playbackTime.TotalSeconds()));
						if (progress >= HitThreshold::ChainSlidePreHitProgress)
							target.HasBeenChainHit = true;
					}
				}
			}
		}

		void CheckUpdateHoldStateMaxOut()
		{
			if (holdState.CurrentHoldTypes != ButtonTypeFlags_None && !holdState.EventHistory.empty())
			{
				const auto lastEvent = holdState.EventHistory.back();
				if (lastEvent.EventType == PlayTestHoldEventType::Start || lastEvent.EventType == PlayTestHoldEventType::Addition)
				{
					const auto timeSinceLastEvent = (GetPlaybackTime() - lastEvent.PlaybackTime);
					if (timeSinceLastEvent >= MaxTargetHoldDuration)
					{
						holdState.EventHistory.push_back({ PlayTestHoldEventType::MaxOut, holdState.CurrentHoldTypes, GetPlaybackTime(), nullptr });
						holdState.CurrentHoldTypes = ButtonTypeFlags_None;
						holdState.ClearAllHeldDownBindings();
					}
				}
			}
		}

		void UpdateInputBindingButtonInputs(const PlayTestInputBinding& binding)
		{
			if (!IsBindingPressed(binding))
				return;

			const auto playbackTime = GetPlaybackTime();
			PlayTestSyncPair* nextPairToHit = FindBestSuitableUnhitSyncPairToEvaluateNext(onScreenTargetPairs, playbackTime);

			bool anyTargetWasHit = false;

			if (nextPairToHit != nullptr)
			{
				for (u8 buttonTypeIndex = 0; buttonTypeIndex < EnumCount<ButtonType>(); buttonTypeIndex++)
				{
					const auto inputButtonType = static_cast<ButtonType>(buttonTypeIndex);
					const auto inputButtonTypeFlag = ButtonTypeToButtonTypeFlags(inputButtonType);

					if ((binding.ButtonTypes & inputButtonTypeFlag) == 0)
						continue;

					const auto remainingTime = nextPairToHit->ButtonTime - playbackTime;

					PlayTestTarget* nextTargetToHit = FindBestSuitableUnhitTargetToEvaluateNext(*nextPairToHit, inputButtonType, binding.SlidePosition);
					if (nextTargetToHit == nullptr)
						continue;

					if (IsSlideButtonType(nextTargetToHit->Type) != IsSlideButtonType(inputButtonType))
						continue;

					if (IsSlideButtonType(nextTargetToHit->Type) && nextTargetToHit->Type != inputButtonType)
					{
						nextTargetToHit->WrongTypeOnTimeOut = true;
						break;
					}

					anyTargetWasHit = true;

					bool inputTypeMatchesAny = false;
					for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
					{
						if (nextPairToHit->Targets[i].Type == inputButtonType)
							inputTypeMatchesAny = true;
					}

					bool matchingType = inputTypeMatchesAny || (nextTargetToHit->Type == inputButtonType);

					if (ButtonTypeToButtonTypeFlags(inputButtonType) != binding.ButtonTypes)
					{
						ButtonTypeFlags inputPairTypeFlags = ButtonTypeFlags_None;
						for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
							inputPairTypeFlags |= ButtonTypeToButtonTypeFlags(nextPairToHit->Targets[i].Type);

						// TODO: Hitting a subsection should also be valid
						if (inputPairTypeFlags != binding.ButtonTypes)
						{
							inputTypeMatchesAny = false;
							matchingType = false;
						}
					}

					const auto coolThreshold = IsSlideButtonType(nextTargetToHit->Type) ? HitThreshold::CoolSlide : HitThreshold::Cool;
					const auto fineThreshold = IsSlideButtonType(nextTargetToHit->Type) ? HitThreshold::FineSlide : HitThreshold::Fine;

					bool successfulHit = false;

					if (matchingType && IsSlideButtonType(nextTargetToHit->Type))
					{
						sharedContext.ButtonSoundController->PlaySlideSound();
						lastSlideActionStopwatch.Restart();
					}

					nextTargetToHit->HasBeenHit = true;
					nextTargetToHit->RemainingTimeOnHit = remainingTime;

					if (remainingTime <= coolThreshold && remainingTime >= -coolThreshold)
					{
						nextTargetToHit->HitEvaluation = matchingType ? HitEvaluation::Cool : HitEvaluation::WrongCool;
						nextTargetToHit->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, coolThreshold);
						successfulHit = true;
					}
					else if (remainingTime <= fineThreshold && remainingTime >= -fineThreshold)
					{
						nextTargetToHit->HitEvaluation = matchingType ? HitEvaluation::Fine : HitEvaluation::WrongFine;
						nextTargetToHit->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, fineThreshold);
						successfulHit = true;
					}
					else if (remainingTime <= HitThreshold::Safe && remainingTime >= -HitThreshold::Safe)
					{
						nextTargetToHit->HitEvaluation = matchingType ? HitEvaluation::Safe : HitEvaluation::WrongSafe;
						nextTargetToHit->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, HitThreshold::Safe);
					}
					else if (remainingTime <= HitThreshold::Sad && remainingTime >= -HitThreshold::Sad)
					{
						nextTargetToHit->HitEvaluation = matchingType ? HitEvaluation::Sad : HitEvaluation::WrongSad;
						nextTargetToHit->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, HitThreshold::Sad);
					}
					else
					{
						nextTargetToHit->HitEvaluation = HitEvaluation::Worst;
						nextTargetToHit->HitPrecision = HitPrecision::Late;
					}

					if (!inputTypeMatchesAny)
					{
						for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
						{
							auto& target = nextPairToHit->Targets[i];
							target.HasBeenHit = true;
							target.RemainingTimeOnHit = remainingTime;
							target.HitEvaluation = nextTargetToHit->HitEvaluation;
							target.HitPrecision = nextTargetToHit->HitPrecision;
						}
					}

					if (inputTypeMatchesAny && successfulHit)
					{
						if (AllInSyncPairHaveBeenHit(*nextPairToHit))
							context.Score.ComboCount++;
					}
					else
					{
						context.Score.ComboCount = 0;
					}

					if (inputTypeMatchesAny)
					{
						if (AllInSyncPairHaveBeenHit(*nextPairToHit))
							ProcessHoldStateForFullyHitSyncPair(*nextPairToHit);

						if (nextTargetToHit->Flags.IsHold)
							holdState.PerTypeHeldDownBindings[static_cast<size_t>(nextTargetToHit->Type)].push_back(&binding);
					}
					else
					{
						const bool inputTypeIsBeingHeld = holdState.CurrentHoldTypes & binding.ButtonTypes;
						if (inputTypeIsBeingHeld)
							ProcessHoldStateCancelNow();
					}
				}
			}

			if ((binding.SlidePosition == SlidePositionType::None))
			{
				if (!anyTargetWasHit)
				{
					for (size_t i = 0; i < EnumCount<ButtonType>(); i++)
					{
						const auto buttonType = static_cast<ButtonType>(i);
						auto& heldDownBindingsForType = holdState.PerTypeHeldDownBindings[i];

						if (heldDownBindingsForType.empty())
							continue;

						if (binding.ButtonTypes & ButtonTypeToButtonTypeFlags(buttonType))
						{
							assert(std::find(heldDownBindingsForType.begin(), heldDownBindingsForType.end(), &binding) == heldDownBindingsForType.end());
							heldDownBindingsForType.push_back(&binding);
						}
					}
				}

				const bool allBindingTypesAreHeldDown = ((holdState.CurrentHoldTypes & binding.ButtonTypes) == binding.ButtonTypes);

				if (anyTargetWasHit || !allBindingTypesAreHeldDown)
					sharedContext.ButtonSoundController->PlayButtonSound();
			}
		}

		void UpdateChainSlideDirection(ButtonType slideDirection, f32 strength, SliderTouchPoint& touchPoint)
		{
			if (lastSlideActionStopwatch.IsRunning() && lastSlideActionStopwatch.GetElapsed() < TimeSpan::FromSeconds(1.0))
			{
				touchPoint.NormalizedPosition = 0.5f;
			}
			else
			{
				touchPoint.NormalizedPosition = glm::clamp(touchPoint.NormalizedPosition + touchPoint.Direction * touchPoint.Speed * strength * Gui::GetIO().DeltaTime, 0.0f, 1.0f);

				if (touchPoint.LastSoundStopwatch.GetElapsed() > touchPoint.SoundInterval)
				{
					const auto touchPointIndex = std::clamp(static_cast<i32>(glm::round(touchPoint.NormalizedPosition * 32.0f)), 0, 31);
					sharedContext.ButtonSoundController->PlaySliderTouch(touchPointIndex, 0.5f);

					touchPoint.LastSoundStopwatch.Restart();
				}
			}

			const auto playbackTime = GetPlaybackTime();

			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid)
					continue;

				for (size_t i = 0; i < onScreenPair.TargetCount; i++)
				{
					auto& onScreenTarget = onScreenPair.Targets[i];
					if (onScreenTarget.Flags.IsChain)
					{
						if (onScreenTarget.Type != slideDirection)
							continue;

						const auto elapsedTime = playbackTime - onScreenPair.TargetTime;
						const auto remainingTime = onScreenPair.ButtonTime - playbackTime;
						const auto progressUnbound = static_cast<f32>(ConvertRange(onScreenPair.TargetTime.TotalSeconds(), onScreenPair.ButtonTime.TotalSeconds(), 0.0, 1.0, playbackTime.TotalSeconds()));

						// TODO: Refine this mechanic, should only work if the chain start fragment has been hit (?) and should ideally continue on the last hit fragment even if strength has decreased
						const auto progressThreshold = (HitThreshold::ChainSlidePreHitProgress / strength);

						if (progressUnbound >= progressThreshold)
						{
							onScreenTarget.HasBeenChainHit = true;
							if (onScreenTarget.Flags.IsChainEnd && !onScreenTarget.HasBeenHit)
								return;
						}
					}
					else if (!onScreenTarget.HasBeenHit)
					{
						return;
					}
				}
			}
		}

		void ProcessHoldStateCancelNow()
		{
			holdState.CurrentHoldTypes = ButtonTypeFlags_None;
			holdState.EventHistory.push_back({ PlayTestHoldEventType::Cancel, holdState.CurrentHoldTypes, GetPlaybackTime(), nullptr });
			holdState.ClearAllHeldDownBindings();
		}

		void ProcessHoldStateForFullyHitSyncPair(const PlayTestSyncPair& syncPair)
		{
			const ButtonTypeFlags pairHoldTypes = GetSyncPairHoldTypeFlags(syncPair);
			const ButtonTypeFlags pairTypes = GetSyncPairTypeFlags(syncPair);

			const bool allHoldTargetsHitByPlayer = AllHoldsInSyncPairHaveBeenCorrectlyHitByPlayer(syncPair);
			const bool typeIsAlreadyBeingHeld = (holdState.CurrentHoldTypes & pairTypes);

			if (pairHoldTypes != ButtonTypeFlags_None && allHoldTargetsHitByPlayer)
			{
				if (typeIsAlreadyBeingHeld || (holdState.CurrentHoldTypes == ButtonTypeFlags_None))
				{
					holdState.CurrentHoldTypes = pairHoldTypes;
					holdState.EventHistory.push_back({ PlayTestHoldEventType::Start, holdState.CurrentHoldTypes, GetPlaybackTime(), &syncPair });

					if (pairHoldTypes == pairTypes)
						holdState.ClearAllHeldDownBindings();
				}
				else
				{
					holdState.CurrentHoldTypes |= pairHoldTypes;
					holdState.EventHistory.push_back({ PlayTestHoldEventType::Addition, holdState.CurrentHoldTypes, GetPlaybackTime(), &syncPair });
				}
			}
			else if (typeIsAlreadyBeingHeld)
			{
				ProcessHoldStateCancelNow();
			}
		}

		TimeSpan GetPlaybackTime() const
		{
#if 1 // NOTE: Same thing applies here as for the chart editor
			return (sharedContext.SongVoice->GetPositionSmooth() - sharedContext.Chart->StartOffset);
#else
			return (sharedContext.SongVoice->GetPosition() - sharedContext.Chart->StartOffset);
#endif
		}

		void SetPlaybackTime(TimeSpan value)
		{
			sharedContext.SongVoice->SetPosition(value + sharedContext.Chart->StartOffset);
		}

	private:
		PlayTestWindow& window;
		PlayTestContext& context;
		PlayTestSharedContext& sharedContext;

	private:
		PlayTestExitType exitRequestThisFrame = PlayTestExitType::None;

		TimeSpan restartPoint = TimeSpan::Zero();
		TimeSpan chartDuration = TimeSpan::FromMinutes(1.0);
		std::vector<PlayTestSyncPair> availableTargetPairs, onScreenTargetPairs;

		PlayTestHoldState holdState = {};

		bool autoplayEnabled = false;

		Stopwatch lastSlideActionStopwatch = {};
		SliderTouchPoint sliderTouchPointL = { -1.0f }, sliderTouchPointR = { +1.0f };

		struct PauseFadeData
		{
			bool PlaybackThisFrame = false, PlaybackLastFrame = false;
			Stopwatch InStopwatch = {}, OutStopwatch = {};

			const TimeSpan Duration = TimeSpan::FromMilliseconds(95.0);
			const f32 Opacity = 0.5f;
		} pauseFade = {};

		struct FadeInOutData
		{
			Stopwatch InStopwatch = {}, OutExitStopwatch = {};
			PlayTestExitType OutExitType = PlayTestExitType::None;
			const TimeSpan InDuration = TimeSpan::FromSeconds(0.75);
			const TimeSpan OutDuration = TimeSpan::FromSeconds(0.20);
		} fadeInOut = {};

		struct MouseHideData
		{
			Stopwatch LastMovementStopwatch = Stopwatch::StartNew();
			const TimeSpan AutoHideThreshold = TimeSpan::FromSeconds(2.0);
		} mouseHide = {};
	};

	PlayTestCore::PlayTestCore(PlayTestWindow& window, PlayTestContext& context, PlayTestSharedContext& sharedContext)
		: impl(std::make_unique<Impl>(window, context, sharedContext))
	{
	}

	PlayTestCore::~PlayTestCore()
	{
	}

	void PlayTestCore::UpdateTick()
	{
		return impl->UpdateTick();
	}

	void PlayTestCore::OverlayGui()
	{
		return impl->OverlayGui();
	}

	PlayTestExitType PlayTestCore::GetAndClearExitRequestThisFrame()
	{
		return impl->GetAndClearExitRequestThisFrame();
	}

	void PlayTestCore::Restart(TimeSpan startTime)
	{
		return impl->Restart(startTime);
	}

	bool PlayTestCore::GetAutoplayEnabled() const
	{
		return impl->GetAutoplayEnabled();
	}

	void PlayTestCore::SetAutoplayEnabled(bool value)
	{
		return impl->SetAutoplayEnabled(value);
	}

	bool PlayTestCore::GetIsPlayback() const
	{
		return impl->GetIsPlayback();
	}
}

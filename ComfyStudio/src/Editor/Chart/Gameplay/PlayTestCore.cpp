#include "PlayTestCore.h"
#include "PlayTestWindow.h"
#include "Time/Stopwatch.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		enum class InputSlideType
		{
			None,
			Left,
			Right,
			Any,
		};

		struct PlayTestInputBinding
		{
			ButtonTypeFlags ButtonTypes;
			InputSlideType SlideType;
			std::variant<Input::KeyCode, Input::DS4Button> InputSource;
		};

		constexpr std::array PlayTestInputBindings =
		{
			PlayTestInputBinding { ButtonTypeFlags_Triangle, InputSlideType::None, Input::KeyCode_W },
			PlayTestInputBinding { ButtonTypeFlags_Square, InputSlideType::None, Input::KeyCode_A },
			PlayTestInputBinding { ButtonTypeFlags_Cross, InputSlideType::None, Input::KeyCode_S },
			PlayTestInputBinding { ButtonTypeFlags_Circle, InputSlideType::None, Input::KeyCode_D },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, InputSlideType::Left, Input::KeyCode_Q },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, InputSlideType::Right, Input::KeyCode_E },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, InputSlideType::None, Input::KeyCode_I },
			PlayTestInputBinding { ButtonTypeFlags_Square, InputSlideType::None, Input::KeyCode_J },
			PlayTestInputBinding { ButtonTypeFlags_Cross, InputSlideType::None, Input::KeyCode_K },
			PlayTestInputBinding { ButtonTypeFlags_Circle, InputSlideType::None, Input::KeyCode_L },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, InputSlideType::Left, Input::KeyCode_U },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, InputSlideType::Right, Input::KeyCode_O },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_1 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_2 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_3 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_4 },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_7 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_8 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_9 },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::KeyCode_0 },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, InputSlideType::None, Input::DS4Button::Triangle },
			PlayTestInputBinding { ButtonTypeFlags_Square, InputSlideType::None, Input::DS4Button::Square },
			PlayTestInputBinding { ButtonTypeFlags_Cross, InputSlideType::None, Input::DS4Button::Cross },
			PlayTestInputBinding { ButtonTypeFlags_Circle, InputSlideType::None, Input::DS4Button::Circle },

			PlayTestInputBinding { ButtonTypeFlags_Triangle, InputSlideType::None, Input::DS4Button::DPad_Up },
			PlayTestInputBinding { ButtonTypeFlags_Square, InputSlideType::None, Input::DS4Button::DPad_Left },
			PlayTestInputBinding { ButtonTypeFlags_Cross, InputSlideType::None, Input::DS4Button::DPad_Down },
			PlayTestInputBinding { ButtonTypeFlags_Circle, InputSlideType::None, Input::DS4Button::DPad_Right },

			PlayTestInputBinding { ButtonTypeFlags_SlideL, InputSlideType::Any, Input::DS4Button::L1 },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, InputSlideType::Any, Input::DS4Button::R1 },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, InputSlideType::Left, Input::DS4Button::L_Stick_Left },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, InputSlideType::Right, Input::DS4Button::L_Stick_Right },
			PlayTestInputBinding { ButtonTypeFlags_SlideL, InputSlideType::Left, Input::DS4Button::R_Stick_Left },
			PlayTestInputBinding { ButtonTypeFlags_SlideR, InputSlideType::Right, Input::DS4Button::R_Stick_Right },

			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::DS4Button::L_Trigger },
			PlayTestInputBinding { ButtonTypeFlags_NormalAll, InputSlideType::None, Input::DS4Button::R_Trigger },
		};

		struct PlayTestTarget
		{
			ButtonType Type;
			TargetFlags Flags;
			TargetProperties Properties;

			TimeSpan RemainingTimeOnHit;
			bool HasBeenHit;
			bool HasBeenChainHit;
			bool HasAnyChainFragmentFailed;
			bool ThisFragmentCausedChainFailure;;
			bool HasTimedOut;
			bool WrongTypeOnTimeOut;

			HitEvaluation HitEvaluation;
			HitPrecision HitPrecision;
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

		constexpr bool AllInSyncPairHaveBeenHit(const PlayTestSyncPair& syncPair)
		{
			for (size_t i = 0; i < syncPair.TargetCount; i++)
			{
				if (!syncPair.Targets[i].HasBeenHit)
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
					newTarget.Flags = sourceTarget.Flags;
					newTarget.Properties = Rules::TryGetProperties(sourceTarget);
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

			if (sharedContext.SongVoice->GetIsPlaying() && autoplayEnabled)
				UpdateAutoplayInput();

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

				if (Gui::MenuItem(sharedContext.SongVoice->GetIsPlaying() ? "Pause" : "Resume", Input::GetKeyCodeName(Input::KeyCode_Space)))
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

				if (Gui::MenuItem("Return to Editor", Input::GetKeyCodeName(Input::KeyCode_Escape)))
					FadeOutThenExit();

#if COMFY_DEBUG
				Gui::Separator();
				if (Gui::BeginMenu("Color Correction"))
				{
					Gui::Checkbox("Enabled", &context.RenderTarget->Param.PostProcessingEnabled);
					Gui::SliderFloat("Saturate", &context.RenderTarget->Param.PostProcessing.Saturation, 2.0f, 3.0f);
					Gui::SliderFloat("Brightness", &context.RenderTarget->Param.PostProcessing.Brightness, 0.35f, 0.80f);

					Gui::SliderFloat3("Coefficients R", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[0]), 0.0f, 1.25f);
					Gui::SliderFloat3("Coefficients G", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[1]), 0.0f, 1.25f);
					Gui::SliderFloat3("Coefficients B", glm::value_ptr(context.RenderTarget->Param.PostProcessing.ColorCoefficientsRGB[2]), 0.0f, 1.25f);

					Gui::EndMenu();
				}
#endif
			});

			if (auto delta = Gui::GetIO().MouseDelta; delta.x != 0.0f || delta.y != 0.0f)
				mouseHide.LastMovementStopwatch.Restart();

			if (!contextMenuOpen && Gui::IsWindowHovered())
			{
				if (mouseHide.LastMovementStopwatch.GetElapsed() > mouseHide.AutoHideThreshold)
					Gui::SetMouseCursor(ImGuiMouseCursor_None);
			}
		}

		bool ExitRequestedThisFrame()
		{
			const bool result = exitRequestedThisFrame;
			exitRequestedThisFrame = false;
			return result;
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
				FadeOutThenExit();

			if (Gui::IsKeyPressed(Input::KeyCode_F1, false))
				autoplayEnabled ^= true;

			if (Gui::IsKeyPressed(Input::KeyCode_Space, false) || Input::DualShock4::IsTapped(Input::DS4Button::Options))
				TogglePause();

			if (Gui::IsKeyPressed(Input::KeyCode_Enter, false) || Input::DualShock4::IsTapped(Input::DS4Button::R3))
				RestartFromRestartPoint();

			if (sharedContext.SongVoice->GetIsPlaying())
			{
				const bool shiftDown = Gui::GetIO().KeyShift;

				if ((shiftDown && Gui::IsKeyPressed(Input::KeyCode_Tab, false)) || Input::DualShock4::IsTapped(Input::DS4Button::L3))
					MoveResetPointBackward();

				if ((!shiftDown && Gui::IsKeyPressed(Input::KeyCode_Tab, false)) || Input::DualShock4::IsTapped(Input::DS4Button::Touch))
					MoveResetPointToPlaybackTime();

				if (!autoplayEnabled)
				{
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
			TargetRenderHelper::BackgroundData backgroundData = {};
			backgroundData.DrawGrid = true;
			backgroundData.DrawDim = true;
			backgroundData.DrawCover = true;
			backgroundData.DrawLogo = true;
			backgroundData.DrawBackground = true;
			backgroundData.PlaybackTime = GetPlaybackTime();
			backgroundData.CoverSprite = sharedContext.Chart->Properties.Image.Cover.GetTexSprView();
			backgroundData.LogoSprite = sharedContext.Chart->Properties.Image.Logo.GetTexSprView();
			backgroundData.BackgroundSprite = sharedContext.Chart->Properties.Image.Background.GetTexSprView();
			sharedContext.RenderHelper->DrawBackground(*sharedContext.Renderer, backgroundData);
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
							onScreenTarget.HasBeenHit = true;
							onScreenTarget.RemainingTimeOnHit = TimeSpan::Zero();
							onScreenTarget.HitEvaluation = HitEvaluation::Cool;

							if (onScreenTarget.Flags.IsChainStart)
							{
								sharedContext.ButtonSoundController->PlayChainSoundStart(chainSlot);
								context.Score.ComboCount++;
								context.Score.ChainSlideScore = 0;
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

								sharedContext.ButtonSoundController->FadeOutLastChainSound(chainSlot);
								sharedContext.ButtonSoundController->PlayChainSoundFailure(chainSlot);
								context.Score.ChainSlideScore = 0;
							}

							onScreenTarget.HasBeenHit = true;
							onScreenTarget.RemainingTimeOnHit = remainingTime;
							onScreenTarget.HasTimedOut = true;
							onScreenTarget.HitEvaluation = HitEvaluation::Worst;
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

					if (onScreenTarget.HasBeenHit && !onScreenTarget.HasTimedOut)
					{
						const auto timeOnHit = onScreenPair.ButtonTime - onScreenTarget.RemainingTimeOnHit;
						const auto timeSinceHit = playbackTime - timeOnHit;

						auto& targetHitData = context.RenderHelperEx.EmplaceTargetHit();
						targetHitData.Position = properties.Position;
						targetHitData.Time = timeSinceHit;
						targetHitData.SlideL = (onScreenTarget.Type == ButtonType::SlideL);
						targetHitData.SlideR = (onScreenTarget.Type == ButtonType::SlideR);
						targetHitData.Chain = onScreenTarget.Flags.IsChain;
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
			pauseFade.PlaybackThisFrame = sharedContext.SongVoice->GetIsPlaying();

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
			if (!sharedContext.SongVoice->GetIsPlaying())
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
					exitRequestedThisFrame = true;
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
			const bool isPlaying = sharedContext.SongVoice->GetIsPlaying();
			PlayOneShotSoundEffect(isPlaying ? "se_ft_sys_dialog_open" : "se_ft_sys_dialog_close");
			sharedContext.SongVoice->SetIsPlaying(!isPlaying);

			if (isPlaying)
			{
				for (size_t i = 0; i < EnumCount<ChainSoundSlot>(); i++)
					sharedContext.ButtonSoundController->FadeOutLastChainSound(static_cast<ChainSoundSlot>(i));
			}
		}

		void FadeOutThenExit()
		{
			if (!fadeInOut.OutExitStopwatch.IsRunning())
				fadeInOut.OutExitStopwatch.Restart();
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
			if (sharedContext.SongVoice->GetIsPlaying())
			{
				for (size_t i = 0; i < EnumCount<ChainSoundSlot>(); i++)
					sharedContext.ButtonSoundController->FadeOutLastChainSound(static_cast<ChainSoundSlot>(i));
			}

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
			const auto hitThreshold = TimeSpan::FromSeconds(Gui::GetIO().DeltaTime * 0.5f);

			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid || AllInSyncPairHaveBeenHit(onScreenPair))
					continue;

				const auto remainingTime = (onScreenPair.ButtonTime - playbackTime);
				if (remainingTime <= hitThreshold)
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
							context.Score.ComboCount++;
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
					if (target.HasBeenHit || !target.Flags.IsChain)
						return;

					const auto progress = static_cast<f32>(ConvertRange(onScreenPair.TargetTime.TotalSeconds(), onScreenPair.ButtonTime.TotalSeconds(), 0.0, 1.0, playbackTime.TotalSeconds()));
					if (progress >= HitThreshold::ChainSlidePreHitProgress)
						target.HasBeenChainHit = true;
				}
			}
		}

		void UpdateInputBindingButtonInputs(const PlayTestInputBinding& binding)
		{
			if (!IsBindingPressed(binding))
				return;

			if (binding.SlideType == InputSlideType::None)
				sharedContext.ButtonSoundController->PlayButtonSound();

			const auto playbackTime = GetPlaybackTime();

			PlayTestSyncPair* nextPairToHit = nullptr;
			for (auto& onScreenPair : onScreenTargetPairs)
			{
				if (onScreenPair.NoLongerValid || AllInSyncPairHaveBeenHit(onScreenPair))
					continue;

				const auto remainingTime = (onScreenPair.ButtonTime - playbackTime);
				if (remainingTime <= HitThreshold::Sad && remainingTime >= -HitThreshold::Worst)
				{
					nextPairToHit = &onScreenPair;
					break;
				}
			}

			if (nextPairToHit == nullptr)
				return;

			for (u8 buttonTypeIndex = 0; buttonTypeIndex < EnumCount<ButtonType>(); buttonTypeIndex++)
			{
				const auto inputButtonType = static_cast<ButtonType>(buttonTypeIndex);
				if ((binding.ButtonTypes & ButtonTypeToButtonTypeFlags(inputButtonType)) == 0)
					continue;

				const auto remainingTime = nextPairToHit->ButtonTime - playbackTime;

				PlayTestTarget* firstUnhitTarget = nullptr;

				if (IsSlideButtonType(inputButtonType))
				{
					for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
					{
						if (!nextPairToHit->Targets[i].HasBeenHit && !nextPairToHit->Targets[i].HasTimedOut && !nextPairToHit->Targets[i].Flags.IsChain && nextPairToHit->Targets[i].Type == inputButtonType)
						{
							firstUnhitTarget = &nextPairToHit->Targets[i];
							break;
						}
					}
				}

				if (firstUnhitTarget == nullptr)
				{
					for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
					{
						if (!nextPairToHit->Targets[i].HasBeenHit && !nextPairToHit->Targets[i].HasTimedOut && !nextPairToHit->Targets[i].Flags.IsChain)
						{
							firstUnhitTarget = &nextPairToHit->Targets[i];
							break;
						}
					}
				}

				if (firstUnhitTarget != nullptr && (IsSlideButtonType(firstUnhitTarget->Type) == IsSlideButtonType(inputButtonType)))
				{
					// TODO: Correctly handle sync slide binding.SlideType
					if (IsSlideButtonType(firstUnhitTarget->Type) && firstUnhitTarget->Type != inputButtonType)
					{
						firstUnhitTarget->WrongTypeOnTimeOut = true;
						break;
					}

					bool inputTypeMatchesAny = false;
					for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
					{
						if (nextPairToHit->Targets[i].Type == inputButtonType)
							inputTypeMatchesAny = true;
					}

					bool matchingType = inputTypeMatchesAny || (firstUnhitTarget->Type == inputButtonType);

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

					const auto coolThreshold = IsSlideButtonType(firstUnhitTarget->Type) ? HitThreshold::CoolSlide : HitThreshold::Cool;
					const auto fineThreshold = IsSlideButtonType(firstUnhitTarget->Type) ? HitThreshold::FineSlide : HitThreshold::Fine;

					bool successfulHit = false;

					if (matchingType && IsSlideButtonType(firstUnhitTarget->Type))
					{
						sharedContext.ButtonSoundController->PlaySlideSound();
						lastSlideActionStopwatch.Restart();
					}

					firstUnhitTarget->HasBeenHit = true;
					firstUnhitTarget->RemainingTimeOnHit = remainingTime;

					if (remainingTime <= coolThreshold && remainingTime >= -coolThreshold)
					{
						firstUnhitTarget->HitEvaluation = matchingType ? HitEvaluation::Cool : HitEvaluation::WrongCool;
						firstUnhitTarget->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, coolThreshold);
						successfulHit = true;
					}
					else if (remainingTime <= fineThreshold && remainingTime >= -fineThreshold)
					{
						firstUnhitTarget->HitEvaluation = matchingType ? HitEvaluation::Fine : HitEvaluation::WrongFine;
						firstUnhitTarget->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, fineThreshold);
						successfulHit = true;
					}
					else if (remainingTime <= HitThreshold::Safe && remainingTime >= -HitThreshold::Safe)
					{
						firstUnhitTarget->HitEvaluation = matchingType ? HitEvaluation::Safe : HitEvaluation::WrongSafe;
						firstUnhitTarget->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, HitThreshold::Safe);
					}
					else if (remainingTime <= HitThreshold::Sad && remainingTime >= -HitThreshold::Sad)
					{
						firstUnhitTarget->HitEvaluation = matchingType ? HitEvaluation::Sad : HitEvaluation::WrongSad;
						firstUnhitTarget->HitPrecision = HitThreshold::EvaluatePrecision(remainingTime, HitThreshold::Sad);
					}
					else
					{
						firstUnhitTarget->HitEvaluation = HitEvaluation::Worst;
						firstUnhitTarget->HitPrecision = HitPrecision::Late;
					}

					if (!inputTypeMatchesAny)
					{
						for (size_t i = 0; i < nextPairToHit->TargetCount; i++)
						{
							auto& target = nextPairToHit->Targets[i];
							target.HasBeenHit = true;
							target.RemainingTimeOnHit = remainingTime;
							target.HitEvaluation = firstUnhitTarget->HitEvaluation;
							target.HitPrecision = firstUnhitTarget->HitPrecision;
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
				}
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

		TimeSpan GetPlaybackTime() const
		{
			return sharedContext.SongVoice->GetPosition() - sharedContext.Chart->StartOffset;
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
		bool exitRequestedThisFrame = false;

		TimeSpan restartPoint = TimeSpan::Zero();
		TimeSpan chartDuration = TimeSpan::FromMinutes(1.0);
		std::vector<PlayTestSyncPair> availableTargetPairs, onScreenTargetPairs;

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
			const TimeSpan InDuration = TimeSpan::FromSeconds(0.75);
			const TimeSpan OutDuration = TimeSpan::FromSeconds(0.20);
		} fadeInOut = {};

		struct MouseHideData
		{
			Stopwatch LastMovementStopwatch = {};
			const TimeSpan AutoHideThreshold = TimeSpan::FromSeconds(3.0);
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

	bool PlayTestCore::ExitRequestedThisFrame()
	{
		return impl->ExitRequestedThisFrame();
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

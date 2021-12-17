#include "PVScriptImportWindow.h"
#include "Core/ComfyStudioSettings.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Input/Input.h"

namespace Comfy::Studio::Editor
{
	static_assert(std::size(PVScriptImportWindow::ImportStatistics::BarDivisionsToCheck) == PVScriptImportWindow::ImportStatistics::BarDivisionsToCheckCount);

	namespace
	{
		void CreateTempoMapApproximationFromDecomposedFlyingTimeCommandsAndImportSettings(const DecomposedPVScriptChartData& decomposedScript, SortedTempoMap& outTempoMap, const PVScriptImportWindow::ImportSettings& settings)
		{
			outTempoMap.Reset();
			outTempoMap.RebuildAccelerationStructure();

			Tempo lastTempo = TempoChange::DefaultTempo;
			f32 lastFlyingTimeFactor = TempoChange::DefaultFlyingTimeFactor.Factor;
			TimeSignature lastSetSignature = TempoChange::DefaultSignature;

			for (const auto& flyingTimeCommand : decomposedScript.FlyingTimeCommands)
			{
				const bool isFirst = (&flyingTimeCommand == &decomposedScript.FlyingTimeCommands[0]);
				const BeatTick newTick = isFirst ? BeatTick::Zero() : Max(outTempoMap.TimeToTick(flyingTimeCommand.CommandTime + settings.TargetOffset), BeatTick::Zero());
				const Tempo newTempo = flyingTimeCommand.RealTempo;
				const f32 newFlyingTimeFactor = static_cast<f32>(static_cast<f64>(flyingTimeCommand.FlyingTimeTempo.BeatsPerMinute) / static_cast<f64>(flyingTimeCommand.RealTempo.BeatsPerMinute));
				const TimeSignature newSignature = flyingTimeCommand.Signature;

				outTempoMap.SetTempoChange(TempoChange(
					newTick,
					(isFirst || newTempo.BeatsPerMinute != lastTempo.BeatsPerMinute) ? newTempo : std::optional<Tempo> {},
					!ApproxmiatelySame(newFlyingTimeFactor, lastFlyingTimeFactor) ? newFlyingTimeFactor : std::optional<FlyingTimeFactor> {},
					(isFirst || newSignature != lastSetSignature) ? newSignature : std::optional<TimeSignature> {}));

				lastTempo = newTempo;
				lastFlyingTimeFactor = newFlyingTimeFactor;
				lastSetSignature = newSignature;

				outTempoMap.RebuildAccelerationStructure();
			}
		}

		void CreateTargetListApproximationFromDecomposedTargetCommandsAndImportSettings(const DecomposedPVScriptChartData& decomposedScript, SortedTempoMap& inTempoMap, SortedTargetList& outTargetList, const PVScriptImportWindow::ImportSettings& settings)
		{
			std::vector<TimelineTarget> outTargets;
			outTargets.reserve(decomposedScript.TargetCommands.size());

			for (const auto& targetCommand : decomposedScript.TargetCommands)
			{
				auto& outTarget = outTargets.emplace_back();
				outTarget.Tick = inTempoMap.TimeToTick(targetCommand.ButtonTime + settings.TargetOffset);
				outTarget.Flags.HasProperties = true;

				auto& outType = outTarget.Type;
				auto& outFlags = outTarget.Flags;

				switch (targetCommand.Parameters.Type)
				{
				case PVCommandLayout::TargetType::Triangle: { outType = ButtonType::Triangle; } break;
				case PVCommandLayout::TargetType::Circle: { outType = ButtonType::Circle; } break;
				case PVCommandLayout::TargetType::Cross: { outType = ButtonType::Cross; } break;
				case PVCommandLayout::TargetType::Square: { outType = ButtonType::Square; } break;

				case PVCommandLayout::TargetType::TriangleHold: { outType = ButtonType::Triangle; outFlags.IsHold = true; } break;
				case PVCommandLayout::TargetType::CircleHold: { outType = ButtonType::Circle; outFlags.IsHold = true; } break;
				case PVCommandLayout::TargetType::CrossHold: { outType = ButtonType::Cross; outFlags.IsHold = true; } break;
				case PVCommandLayout::TargetType::SquareHold: { outType = ButtonType::Square; outFlags.IsHold = true; } break;

				case PVCommandLayout::TargetType::SlideL: { outType = ButtonType::SlideL; } break;
				case PVCommandLayout::TargetType::SlideR: { outType = ButtonType::SlideR; } break;
				case PVCommandLayout::TargetType::SlideChainL: { outType = ButtonType::SlideL; outFlags.IsChain = true; } break;
				case PVCommandLayout::TargetType::SlideChainR: { outType = ButtonType::SlideR; outFlags.IsChain = true; } break;

				case PVCommandLayout::TargetType::TriangleChance: { outType = ButtonType::Triangle; outFlags.IsChance = true; } break;
				case PVCommandLayout::TargetType::CircleChance: { outType = ButtonType::Circle; outFlags.IsChance = true; } break;
				case PVCommandLayout::TargetType::CrossChance: { outType = ButtonType::Cross; outFlags.IsChance = true; } break;
				case PVCommandLayout::TargetType::SquareChance: { outType = ButtonType::Square; outFlags.IsChance = true; } break;
				case PVCommandLayout::TargetType::SlideLChance: { outType = ButtonType::SlideL; outFlags.IsChance = true; } break;
				case PVCommandLayout::TargetType::SlideRChance: { outType = ButtonType::SlideR; outFlags.IsChance = true; } break;
				}

				outTarget.Properties.Position.x = static_cast<f32>(targetCommand.Parameters.PositionX / PVTargetCommandFixedPointPositionFactor);
				outTarget.Properties.Position.y = static_cast<f32>(targetCommand.Parameters.PositionY / PVTargetCommandFixedPointPositionFactor);
				outTarget.Properties.Angle = static_cast<f32>(targetCommand.Parameters.Angle / PVTargetCommandFixedPointAngleFactor);
				outTarget.Properties.Frequency = static_cast<f32>(targetCommand.Parameters.Frequency);
				outTarget.Properties.Amplitude = static_cast<f32>(targetCommand.Parameters.Amplitude);
				outTarget.Properties.Distance = static_cast<f32>(targetCommand.Parameters.Distance / PVTargetCommandFixedPointPositionFactor);
			}

			outTargetList.Clear();
			outTargetList = std::move(outTargets);
			for (auto& outTarget : outTargetList)
			{
				if (outTarget.Flags.IsChain && !outTarget.Flags.IsChainStart)
					outTarget.Properties.Position.x -= Rules::ChainFragmentStartEndOffsetDistance * (outTarget.Type == ButtonType::SlideL ? -1.0f : +1.0f);
			}
		}

		std::unique_ptr<Chart> CreateChartFromDecomposedScriptAndImportSettings(const DecomposedPVScriptChartData& decomposedScript, std::string_view songPath, std::string_view moviePath, const PVScriptImportWindow::ImportSettings& settings)
		{
			auto outChart = std::make_unique<Chart>();
			outChart->Properties.Song.Title = decomposedScript.ScriptFileName;
			outChart->Properties.Difficulty.Type = decomposedScript.DecomposedScriptName.DifficultyType;
			outChart->Properties.Difficulty.Level = DifficultyLevel::Star_07_5;
			outChart->SongFileName = songPath;
			outChart->MovieFileName = moviePath;

			if (decomposedScript.MusicPlayCommandTime.has_value())
				outChart->SongOffset = -(decomposedScript.MusicPlayCommandTime.value() + settings.TargetOffset);

			if (decomposedScript.MoviePlayCommandTime.has_value())
				outChart->MovieOffset = -(decomposedScript.MoviePlayCommandTime.value() + settings.TargetOffset);

			// NOTE: Just to avoid ugly neagtive zeros in the chart editor sync window later on
			if (outChart->SongOffset == -TimeSpan::Zero()) outChart->SongOffset = TimeSpan::Zero();
			if (outChart->MovieOffset == -TimeSpan::Zero()) outChart->MovieOffset = TimeSpan::Zero();

			outChart->Duration = (decomposedScript.PVEndCommandTime + settings.TargetOffset);

			CreateTempoMapApproximationFromDecomposedFlyingTimeCommandsAndImportSettings(decomposedScript, outChart->TempoMap, settings);
			CreateTargetListApproximationFromDecomposedTargetCommandsAndImportSettings(decomposedScript, outChart->TempoMap, outChart->Targets, settings);
			return outChart;
		}

		TimeSpan CalculateFirstTargetBeatAlignmentTimeOffset(const Chart& chart)
		{
			if (chart.Targets.size() <= 0)
				return TimeSpan::Zero();

			auto roundTickToBeat = [](BeatTick tick)
			{
				const f64 roundTicks = static_cast<f64>(BeatTick::TicksPerBeat);
				return BeatTick::FromTicks(static_cast<i32>(glm::round(static_cast<f64>(tick.Ticks()) / roundTicks) * roundTicks));
			};

			const auto& firstTarget = chart.Targets[0];

			const TimeSpan firstTargetButtonTime = chart.TempoMap.TickToTime(firstTarget.Tick);
			const TimeSpan firstTargetButtonTimeBeatRounded = chart.TempoMap.TickToTime(roundTickToBeat(firstTarget.Tick));

			return (firstTargetButtonTimeBeatRounded - firstTargetButtonTime);
		}

		bool AllFlyingTimeCommandTemposMatchFlyingTime(const std::vector<DecomposedPVScriptChartData::FlyingTimeCommandData>& flyingTimeCommands)
		{
			return std::all_of(flyingTimeCommands.begin(), flyingTimeCommands.end(), [](const auto& f) { return ApproxmiatelySame(f.RealTempo.BeatsPerMinute, f.FlyingTimeTempo.BeatsPerMinute); });
		}

		PVScriptImportWindow::ImportStatistics CalculateImportStatistics(const DecomposedPVScriptChartData& decomposedScript, const Chart& importedChart, const PVScriptImportWindow::ImportSettings& settings)
		{
			assert(importedChart.Targets.size() == decomposedScript.TargetCommands.size());
			PVScriptImportWindow::ImportStatistics out = {};

			// NOTE: The target order between the original and the imported chart within sync pairs might not be the same
			//		 however that shouldn't matter for time comparisons only
			for (size_t i = 0; i < importedChart.Targets.size(); i++)
			{
				const auto& sourceTargetCommand = decomposedScript.TargetCommands[i];
				const auto& convertedTarget = importedChart.Targets[i];

				const TimeSpan sourceButtonTime = sourceTargetCommand.ButtonTime + settings.TargetOffset;
				const TimeSpan sourceTargetTime = sourceTargetCommand.TargetTime + settings.TargetOffset;

				const auto convertedSpawnTimes = importedChart.TempoMap.GetTargetSpawnTimes(convertedTarget);

				const TimeSpan buttonTimeDifference = sourceButtonTime - convertedSpawnTimes.ButtonTime;
				const TimeSpan targetTimeDifference = sourceTargetTime - convertedSpawnTimes.TargetTime;

				if (i == 0)
				{
					out.ButtonTimeDifference.Min = out.ButtonTimeDifference.Max = buttonTimeDifference;
					out.TargetTimeDifference.Min = out.TargetTimeDifference.Max = targetTimeDifference;
				}
				else
				{
					out.ButtonTimeDifference.Min = Min(out.ButtonTimeDifference.Min, buttonTimeDifference);
					out.ButtonTimeDifference.Max = Max(out.ButtonTimeDifference.Max, buttonTimeDifference);

					out.TargetTimeDifference.Min = Min(out.TargetTimeDifference.Min, targetTimeDifference);
					out.TargetTimeDifference.Max = Max(out.TargetTimeDifference.Max, targetTimeDifference);
				}

				out.ButtonTimeDifference.Average += buttonTimeDifference;
				out.TargetTimeDifference.Average += targetTimeDifference;

				for (size_t i = 0; i < out.BarDivisionsToCheckCount; i++)
				{
					const i32 barDivision = out.BarDivisionsToCheck[i];
					const i32 divisionTicks = (BeatTick::FromBars(1) / barDivision).Ticks();

					if ((convertedTarget.Tick.Ticks() % divisionTicks) == 0)
					{
						out.BarDivisionDistribution[i]++;
						break;
					}
				}
			}

			out.ButtonTimeDifference.Average = TimeSpan::FromSeconds(out.ButtonTimeDifference.Average.TotalSeconds() / static_cast<f64>(importedChart.Targets.size()));
			out.TargetTimeDifference.Average = TimeSpan::FromSeconds(out.TargetTimeDifference.Average.TotalSeconds() / static_cast<f64>(importedChart.Targets.size()));

			return out;
		}
	}

	void PVScriptImportWindow::Gui()
	{
		StartOfFrameCheckForAndHandleAsyncLoadCompletion();

		// NOTE: Check once at the start of the frame to avoid any mid-frame discrepancies
		const bool isAsyncLoadingAtStartOfFrame = IsAsyncLoading();

		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Gui::GetStyle().WindowPadding);

		Gui::BeginChild("InputBaseChild", vec2(640.0f + 120.0f, 340.0f), true, ImGuiWindowFlags_None);
		{
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Input Script Path");

			Gui::PushItemDisabledAndTextColor();
			{
				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				Gui::PathInputTextWithHint("##ScriptPath", "Script Path...", &inScript.Sync.ScriptPath, ImGuiInputTextFlags_ReadOnly);
				Gui::PopItemWidth();
				Gui::Separator();

				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				Gui::PathInputTextWithHint("##SongPath", "Song Path...", isAsyncLoadingAtStartOfFrame ? &emptyDummyStringToReferenceWhileAsyncLoading : &inScript.Async.SongPath, ImGuiInputTextFlags_ReadOnly);
				Gui::PopItemWidth();
				Gui::Separator();

				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				Gui::PathInputTextWithHint("##MoviePath", "Movie Path...", isAsyncLoadingAtStartOfFrame ? &emptyDummyStringToReferenceWhileAsyncLoading : &inScript.Async.MoviePath, ImGuiInputTextFlags_ReadOnly);
				Gui::PopItemWidth();
				Gui::Separator();
			}
			Gui::PopItemDisabledAndTextColor();

			Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetColorU32(ImGuiCol_WindowBg));

			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("PV Script Source");
			Gui::NextColumn();
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Tempo Map Approximation");
			Gui::NextColumn();
			Gui::Separator();

			Gui::BeginChild("PVScriptSourceChild", vec2(Gui::GetContentRegionAvail().x, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
			if (!isAsyncLoadingAtStartOfFrame && inScript.Async.LoadedScriptFile != nullptr)
			{
				SyntaxHighlightedPVCommandListViewGui(inScript.Async.LoadedScriptFile->Commands);
			}
			Gui::EndChild();

			Gui::NextColumn();

			Gui::BeginChild("TempoMapChild", vec2(Gui::GetContentRegionAvail().x, 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			if (!isAsyncLoadingAtStartOfFrame && !inScript.Async.LoadedScriptFile->Commands.empty() && outChart.ImportedChart != nullptr)
			{
				for (auto& flyingTimeCommand : inScript.Async.DecomposedScipt.FlyingTimeCommands)
				{
					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.TimeCommand);
					// TODO: Make editable as well (?)
					Gui::TextUnformatted(flyingTimeCommand.CommandTime.FormatTime().data());
					Gui::SameLine(0.0f, 0.0f);
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					Gui::TextUnformatted(": (");
					Gui::SameLine();
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Parameters);
					Gui::PushStyleColor(ImGuiCol_FrameBg, Gui::GetColorU32(ImGuiCol_FrameBg, 0.5f));
					Gui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 1.0f });
					{
						constexpr f32 tempoInputFloatWidth = 54.0f;
						const bool tempoMatchesFlyingTime = ApproxmiatelySame(flyingTimeCommand.RealTempo.BeatsPerMinute, flyingTimeCommand.FlyingTimeTempo.BeatsPerMinute);

						Gui::Text("Tempo:");
						Gui::SameLine();

						Gui::PushID(&flyingTimeCommand.RealTempo);
						Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
						Gui::SetNextItemWidth(tempoInputFloatWidth);
						if (Tempo newTempo = flyingTimeCommand.RealTempo; Gui::InputFloat("##RealTempo", &newTempo.BeatsPerMinute, 0.0f, 0.0f, "%.8g BPM"))
						{
							newTempo = Clamp(newTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM);
							if (!ApproxmiatelySame(newTempo.BeatsPerMinute, flyingTimeCommand.RealTempo.BeatsPerMinute))
							{
								flyingTimeCommand.RealTempo = newTempo;
								OnGuiTempoMapValueChanged();
							}
						}
						Gui::PopStyleColor();
						Gui::PopID();
						Gui::SameLine();

						Gui::Text(", Flying Time:");
						Gui::SameLine();

						Gui::PushID(&flyingTimeCommand.FlyingTimeTempo);
						Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
						Gui::SetNextItemWidth(tempoInputFloatWidth);
						if (Tempo newFlyingTempo = flyingTimeCommand.FlyingTimeTempo; Gui::InputFloat("##FlyingTimeTempo", &newFlyingTempo.BeatsPerMinute, 0.0f, 0.0f, tempoMatchesFlyingTime ? "--- BPM" : "%.8g BPM"))
						{
							newFlyingTempo = Clamp(newFlyingTempo.BeatsPerMinute, Tempo::MinBPM, Tempo::MaxBPM);
							if (!ApproxmiatelySame(newFlyingTempo.BeatsPerMinute, flyingTimeCommand.FlyingTimeTempo.BeatsPerMinute))
							{
								flyingTimeCommand.FlyingTimeTempo = newFlyingTempo;
								OnGuiTempoMapValueChanged();
							}
						}
						Gui::PopStyleColor();
						Gui::PopID();
						Gui::SameLine();
					}
					Gui::PopStyleVar(1);
					Gui::PopStyleColor(2);

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					Gui::TextUnformatted(", ");
					Gui::SameLine();
					Gui::PopStyleColor();

					// TODO: Make editable as well (?)
					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Parameters);
					Gui::Text("%d/%d", flyingTimeCommand.Signature.Numerator, flyingTimeCommand.Signature.Denominator);
					Gui::SameLine();
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					Gui::TextUnformatted(")");
					Gui::PopStyleColor();

					Gui::Separator();
				}
			}
			Gui::EndChild();

			Gui::PopStyleColor();
			Gui::EndColumns();
		}
		Gui::EndChild();

		Gui::BeginChild("SettingsBaseChild", vec2(0.0f, 328.0f), true, ImGuiWindowFlags_None);
		{
			Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetColorU32(ImGuiCol_WindowBg));

			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Import Settings");
			Gui::NextColumn();
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Import Statistics");
			Gui::NextColumn();
			Gui::Separator();

			Gui::BeginChild("ImportSettingsChild", vec2(Gui::GetContentRegionAvail().x, 0.0f), true, ImGuiWindowFlags_None);
			Gui::PushItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			{
				Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Target Offset");
				Gui::NextColumn();
				Gui::PushItemWidth(Gui::GetContentRegionAvail().x);
				if (f64 ms = importSettings.TargetOffset.TotalMilliseconds(); Gui::InputDouble("##TargetOffset", &ms, 1.0f, 10.0f, "%.4f ms"))
				{
					importSettings.TargetOffset = TimeSpan::FromMilliseconds(ms);
					OnGuiImportSettingsValueChanged();
				}
				Gui::PopItemWidth();
				Gui::Separator();
				Gui::NextColumn();

				Gui::NextColumn();

				const bool setBeatAlignedDisabled = isAsyncLoadingAtStartOfFrame || ApproxmiatelySame(importSettings.TargetOffset.TotalMilliseconds(), inScript.Async.FirstTargetBeatAlignmentOffset.TotalMilliseconds());
				Gui::PushItemDisabledAndTextColorIf(setBeatAlignedDisabled);
				if (Gui::Button("Beat-Align First Target", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				{
					importSettings.TargetOffset = inScript.Async.FirstTargetBeatAlignmentOffset;
					OnGuiImportSettingsValueChanged();
				}
				Gui::PopItemDisabledAndTextColorIf(setBeatAlignedDisabled);
				Gui::NextColumn();
				Gui::Separator();

				Gui::NextColumn();

				const bool setZeroAlignedDisabled = isAsyncLoadingAtStartOfFrame || (importSettings.TargetOffset == TimeSpan::Zero());
				Gui::PushItemDisabledAndTextColorIf(setZeroAlignedDisabled);
				if (Gui::Button("Zero-Align First Target", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				{
					importSettings.TargetOffset = TimeSpan::Zero();
					OnGuiImportSettingsValueChanged();
				}
				Gui::PopItemDisabledAndTextColorIf(setZeroAlignedDisabled);
				Gui::NextColumn();
				Gui::Separator();

				Gui::InvisibleButton("##Padding", vec2(1.0f, Gui::GetTextLineHeight()));
				Gui::NextColumn();
				Gui::NextColumn();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Tempo Map");
				Gui::NextColumn();

				if (Gui::Button("Reset Approximation", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				{
					inScript.Async.DecomposedScipt.FlyingTimeCommands = inScript.Async.DecomposedSciptUneditedCopy.FlyingTimeCommands;
					OnGuiTempoMapValueChanged();
				}
				Gui::NextColumn();
				Gui::Separator();

				Gui::NextColumn();

				const bool allTemposMatchFlyingTime = isAsyncLoadingAtStartOfFrame || AllFlyingTimeCommandTemposMatchFlyingTime(inScript.Async.DecomposedScipt.FlyingTimeCommands);
				Gui::PushItemDisabledAndTextColorIf(allTemposMatchFlyingTime);
				{
					if (Gui::Button("Set Tempo to Flying Time", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
					{
						for (auto& flyingTimeCommand : inScript.Async.DecomposedScipt.FlyingTimeCommands)
							flyingTimeCommand.RealTempo = flyingTimeCommand.FlyingTimeTempo;
						OnGuiTempoMapValueChanged();
					}
					Gui::NextColumn();
					Gui::Separator();

					Gui::NextColumn();

					if (Gui::Button("Set Flying Time to Tempo", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
					{
						for (auto& flyingTimeCommand : inScript.Async.DecomposedScipt.FlyingTimeCommands)
							flyingTimeCommand.FlyingTimeTempo = flyingTimeCommand.RealTempo;
						OnGuiTempoMapValueChanged();
					}
					Gui::NextColumn();
					Gui::Separator();
				}
				Gui::PopItemDisabledAndTextColorIf(allTemposMatchFlyingTime);

				Gui::EndColumns();

				// TODO: Help text to explain flying time, optimization of button time precision and a large flying time precision loss being a indication for a misconfigured flying time
				//		 and button to brute force a flying time factor for lowst target time precision loss
			}
			Gui::PopItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			Gui::EndChild();

			Gui::NextColumn();

			Gui::BeginChild("ImportStatisticsChild", vec2(Gui::GetContentRegionAvail().x, 0.0f), true, ImGuiWindowFlags_None);
			Gui::PushItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			{
				const ImVec4 paramTextColor = Gui::ColorConvertU32ToFloat4(PVCommandListViewSyntaxColors.Parameters);

				// TODO: Color lerp white (or green) to red if precision loss too high (?)
				{
					Gui::TextUnformatted("Time Precision Loss");
					Gui::Separator();

					Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
					Gui::TextColored(paramTextColor, "Min, Max, Average");
					Gui::NextColumn();
					Gui::Text("%.2f ms, %.2f ms, %.2f ms",
						importStatistics.ButtonTimeDifference.Min.TotalMilliseconds(),
						importStatistics.ButtonTimeDifference.Max.TotalMilliseconds(),
						importStatistics.ButtonTimeDifference.Average.TotalMilliseconds());
					Gui::EndColumns();
				}
				Gui::Separator();
				Gui::InvisibleButton("##Padding", vec2(1.0f, Gui::GetTextLineHeight()));
				{
					Gui::TextUnformatted("Flying Time Precision Loss (Visual Only)");
					Gui::Separator();

					Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
					Gui::TextColored(paramTextColor, "Min, Max, Average");
					Gui::NextColumn();
					Gui::Text("%.2f ms, %.2f ms, %.2f ms",
						importStatistics.TargetTimeDifference.Min.TotalMilliseconds(),
						importStatistics.TargetTimeDifference.Max.TotalMilliseconds(),
						importStatistics.TargetTimeDifference.Average.TotalMilliseconds());
					Gui::EndColumns();
				}
				Gui::Separator();
				Gui::InvisibleButton("##Padding", vec2(1.0f, Gui::GetTextLineHeight()));
				{
					Gui::TextUnformatted("Target Distribution");
					Gui::Separator();

					// TODO: Color lerp to red if too many "unusual" targets (?)
					Gui::BeginColumns(nullptr, 2, ImGuiOldColumnFlags_NoResize);
					for (size_t i = 0; i < importStatistics.BarDivisionsToCheckCount; i++)
					{
						const i32 barDivision = importStatistics.BarDivisionsToCheck[i];
						const size_t targetCount = importStatistics.BarDivisionDistribution[i];

						Gui::TextColored(paramTextColor, "Targets at 1 / %d", barDivision);
						Gui::NextColumn();

						Gui::Text("%zu", targetCount);
						Gui::NextColumn();

						Gui::Separator();
					}
					Gui::EndColumns();
				}
			}
			Gui::PopItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			Gui::EndChild();

			Gui::PopStyleColor();
			Gui::EndColumns();
		}
		Gui::EndChild();

		Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetColorU32(ImGuiCol_WindowBg));
		Gui::BeginChild("ConfirmatioBaseChild", vec2(0.0f, 32.0f), true, ImGuiWindowFlags_None);
		{
			if (Gui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !thisFrameAnyItemActive && !lastFrameAnyItemActive)
			{
				if (!isAsyncLoadingAtStartOfFrame && Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
					RequestExitAndImport();
				else if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
					RequestExitWithoutImport();
			}

			Gui::PushItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			if (Gui::Button("Import", vec2((Gui::GetContentRegionAvail().x - Gui::GetStyle().ItemSpacing.x) * 0.5f, Gui::GetContentRegionAvail().y)))
				RequestExitAndImport();
			Gui::PopItemDisabledAndTextColorIf(isAsyncLoadingAtStartOfFrame);
			Gui::SameLine();
			if (Gui::Button("Cancel", Gui::GetContentRegionAvail()))
				RequestExitWithoutImport();
		}
		Gui::EndChild();
		Gui::PopStyleColor();

		Gui::PopStyleVar();
	}

	void PVScriptImportWindow::SetScriptFilePathAndStartAsyncLoading(std::string_view scriptPath)
	{
		WaitForAsyncLoadCompletion();
		inScript.Sync = {};
		inScript.Async = {};

		inScript.Sync.ScriptPath = IO::Path::Normalize(scriptPath);
		if (inScript.Sync.ScriptPath.empty())
			return;

		importSettings = {};
		importStatistics = {};

		isFirstFrameAfterStartingAsyncLoad = true;
		inScript.LoadAndDecomposeChartFileFuture = std::async(std::launch::async, [this]()
		{
			inScript.Async.LoadedScriptFile = IO::File::Load<PVScript>(inScript.Sync.ScriptPath);
			if (inScript.Async.LoadedScriptFile == nullptr)
				inScript.Async.LoadedScriptFile = std::make_unique<PVScript>();

			inScript.Async.DecomposedScipt = DecomposePVScriptChartData(*inScript.Async.LoadedScriptFile, inScript.Sync.ScriptPath);
			inScript.Async.DecomposedSciptUneditedCopy = inScript.Async.DecomposedScipt;

			const std::string_view scriptFileName = IO::Path::GetFileName(inScript.Sync.ScriptPath);
			const auto potentialSongAndMoviePaths = GetPotentialSongAndMovieFilePathsFromPVScriptPath(inScript.Sync.ScriptPath);

			if (inScript.Async.DecomposedScipt.MusicPlayCommandTime.has_value())
			{
				const std::string* firstExistingSongPath = FindIfOrNull(potentialSongAndMoviePaths.SongPaths, [](const auto& p) { return IO::File::Exists(p); });
				inScript.Async.SongPath = (firstExistingSongPath != nullptr) ? *firstExistingSongPath : IO::Path::ChangeExtension(scriptFileName, ".ogg");
			}
			if (inScript.Async.DecomposedScipt.MoviePlayCommandTime.has_value())
			{
				const std::string* firstExistingMoviePath = FindIfOrNull(potentialSongAndMoviePaths.MoviePaths, [](const auto& p) { return IO::File::Exists(p); });
				inScript.Async.MoviePath = (firstExistingMoviePath != nullptr) ? *firstExistingMoviePath : IO::Path::ChangeExtension(scriptFileName, ".mp4");
			}
		});
	}

	bool PVScriptImportWindow::GetAndClearCloseRequestThisFrame()
	{
		const bool result = closeWindowThisFrame;
		closeWindowThisFrame = false;
		return result;
	}

	std::unique_ptr<Chart> PVScriptImportWindow::TryMoveImportedChartBeforeClosing()
	{
		auto movedImportedChart = std::move(outChart.ImportedChart);

		// NOTE: The rest of the "cleanup" happens the next time the window is opened, to not stall for async loading
		outChart = {};

		return movedImportedChart;
	}

	bool PVScriptImportWindow::IsAsyncLoading() const
	{
		return inScript.LoadAndDecomposeChartFileFuture.valid();
	}

	void PVScriptImportWindow::WaitForAsyncLoadCompletion()
	{
		if (inScript.LoadAndDecomposeChartFileFuture.valid())
			inScript.LoadAndDecomposeChartFileFuture.get();
	}

	void PVScriptImportWindow::StartOfFrameCheckForAndHandleAsyncLoadCompletion()
	{
		if (isFirstFrameAfterStartingAsyncLoad)
		{
			// HACK: Hacky way of potentially giving the async worker thread just enough time to finish loading the script file and therefore avoid showing a single "disabled" frame
			if (inScript.LoadAndDecomposeChartFileFuture.valid())
				inScript.LoadAndDecomposeChartFileFuture.wait_for(std::chrono::milliseconds(2));

			isFirstFrameAfterStartingAsyncLoad = false;
		}

		if (inScript.LoadAndDecomposeChartFileFuture.valid() && inScript.LoadAndDecomposeChartFileFuture._Is_ready())
		{
			inScript.LoadAndDecomposeChartFileFuture.get();
			importSettings.TargetOffset = TimeSpan::Zero();
			{
				CreateChartFromDecomposedScriptAssignToImportedChart();
				RecalculateFirstTargetBeatAlignmentOffset();
			}
			importSettings.TargetOffset = inScript.Async.FirstTargetBeatAlignmentOffset;
			CreateChartFromDecomposedScriptAssignToImportedChart();
			RecalculateImportStatistics();
		}
	}

	void PVScriptImportWindow::CreateChartFromDecomposedScriptAssignToImportedChart()
	{
		// NOTE: It shouldn't be allowed to input a stalling action while async loading
		assert(!IsAsyncLoading());
		WaitForAsyncLoadCompletion();

		outChart.ImportedChart = CreateChartFromDecomposedScriptAndImportSettings(inScript.Async.DecomposedScipt, inScript.Async.SongPath, inScript.Async.MoviePath, importSettings);
	}

	void PVScriptImportWindow::RecalculateFirstTargetBeatAlignmentOffset()
	{
		const auto importSettingsCopy = importSettings;
		{
			importSettings.TargetOffset = TimeSpan::Zero();
			inScript.Async.FirstTargetBeatAlignmentOffset = CalculateFirstTargetBeatAlignmentTimeOffset(*outChart.ImportedChart);
		}
		importSettings = importSettingsCopy;
	}

	void PVScriptImportWindow::RecalculateImportStatistics()
	{
		importStatistics = CalculateImportStatistics(inScript.Async.DecomposedScipt, *outChart.ImportedChart, importSettings);
	}

	void PVScriptImportWindow::OnGuiImportSettingsValueChanged()
	{
		CreateChartFromDecomposedScriptAssignToImportedChart();
		RecalculateImportStatistics();
	}

	void PVScriptImportWindow::OnGuiTempoMapValueChanged()
	{
		const bool targetOffsetWasBeatAligned = ApproxmiatelySame(importSettings.TargetOffset.TotalSeconds(), inScript.Async.FirstTargetBeatAlignmentOffset.TotalSeconds());

		const auto importSettingsCopy = importSettings;
		importSettings.TargetOffset = TimeSpan::Zero();
		{
			CreateChartFromDecomposedScriptAssignToImportedChart();
			RecalculateFirstTargetBeatAlignmentOffset();
		}
		importSettings = importSettingsCopy;

		if (targetOffsetWasBeatAligned)
			importSettings.TargetOffset = inScript.Async.FirstTargetBeatAlignmentOffset;

		CreateChartFromDecomposedScriptAssignToImportedChart();
		RecalculateImportStatistics();
	}

	void PVScriptImportWindow::RequestExitAndImport()
	{
		closeWindowThisFrame = true;

		assert(!IsAsyncLoading());
		WaitForAsyncLoadCompletion();
		// NOTE: The imported chart is expected to always be valid while the window is open
		// outChart.ImportedChart = ...;
	}

	void PVScriptImportWindow::RequestExitWithoutImport()
	{
		closeWindowThisFrame = true;
		outChart.ImportedChart = nullptr;
	}
}

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
		void CreateTempoMapApproximationFromPVCommands(const DecomposedPVScriptChartData& decomposedScript, SortedTempoMap& outTempoMap, TimelineMap& outTimelineMap, const PVScriptImportWindow::ImportSettings& settings)
		{
			outTempoMap.Clear();
			outTimelineMap.CalculateMapTimes(outTempoMap);

			const f32 flyingTimeFactor = (settings.FlyingTimeFactor <= 0.001f) ? 1.0f : settings.FlyingTimeFactor;

			for (const auto& flyingTimeCommand : decomposedScript.FlyingTimeCommands)
			{
				const bool isFirst = (&flyingTimeCommand == &decomposedScript.FlyingTimeCommands[0]);

				outTempoMap.SetTempoChange(
					isFirst ? BeatTick::Zero() : outTimelineMap.GetTickAt(flyingTimeCommand.CommandTime + settings.TargetOffset),
					Tempo(flyingTimeCommand.FlyingTempo.BeatsPerMinute * flyingTimeFactor),
					flyingTimeCommand.Signature);
				outTimelineMap.CalculateMapTimes(outTempoMap);
			}
		}

		void CreateTargetListApproximationFromPVCommands(const DecomposedPVScriptChartData& decomposedScript, const TimelineMap& inTimelineMap, SortedTargetList& outTargetList, const PVScriptImportWindow::ImportSettings& settings)
		{
			outTargetList.Clear();
			std::vector<TimelineTarget> outTargets;

			for (const auto& targetCommand : decomposedScript.TargetCommands)
			{
				auto& outTarget = outTargets.emplace_back();
				outTarget.Tick = inTimelineMap.GetTickAt(targetCommand.ButtonTime + settings.TargetOffset);
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

			outTargetList = std::move(outTargets);
			for (auto& outTarget : outTargetList)
			{
				if (outTarget.Flags.IsChain && !outTarget.Flags.IsChainStart)
					outTarget.Properties.Position.x -= Rules::ChainFragmentStartEndOffsetDistance * (outTarget.Type == ButtonType::SlideL ? -1.0f : +1.0f);
			}
		}

		std::unique_ptr<Chart> CreateImportChartFromDecomposedScript(const DecomposedPVScriptChartData& decomposedScript, std::string_view songPath, const PVScriptImportWindow::ImportSettings& settings)
		{
			auto outChart = std::make_unique<Chart>();
			outChart->Properties.Song.Title = decomposedScript.ScriptFileName;
			outChart->Properties.Difficulty.Type = decomposedScript.DecomposedScriptName.DifficultyType;
			outChart->Properties.Difficulty.Level = DifficultyLevel::Star_07_5;
			outChart->SongFileName = songPath;

			outChart->StartOffset = -(decomposedScript.MusicPlayCommandTime + settings.TargetOffset);

			// NOTE: To avoid an ugly neagtive zero in the chart editor sync window later on
			if (outChart->StartOffset == -TimeSpan::Zero())
				outChart->StartOffset = TimeSpan::Zero();

			outChart->Duration = decomposedScript.PVEndCommandTime + settings.TargetOffset;

			CreateTempoMapApproximationFromPVCommands(decomposedScript, outChart->TempoMap, outChart->TimelineMap, settings);
			CreateTargetListApproximationFromPVCommands(decomposedScript, outChart->TimelineMap, outChart->Targets, settings);
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

			const TimeSpan firstTargetButtonTime = chart.TimelineMap.GetTimeAt(firstTarget.Tick);
			const TimeSpan firstTargetButtonTimeBeatRounded = chart.TimelineMap.GetTimeAt(roundTickToBeat(firstTarget.Tick));

			return (firstTargetButtonTimeBeatRounded - firstTargetButtonTime);
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

				const TimeSpan convertedButtonTime = importedChart.TimelineMap.GetTimeAt(convertedTarget.Tick);
				const TimeSpan convertedTargetTime = importedChart.TimelineMap.GetTimeAt(convertedTarget.Tick - BeatTick::FromBars(1));

				const TimeSpan buttonTimeDifference = sourceButtonTime - convertedButtonTime;
				const TimeSpan targetTimeDifference = sourceTargetTime - convertedTargetTime;

				if (i == 0)
				{
					out.ButtonTimeDifference.Min = out.ButtonTimeDifference.Max = buttonTimeDifference;
					out.TargetTimeDifference.Min = out.TargetTimeDifference.Max = targetTimeDifference;
				}
				else
				{
					out.ButtonTimeDifference.Min = std::min(out.ButtonTimeDifference.Min, buttonTimeDifference);
					out.ButtonTimeDifference.Max = std::max(out.ButtonTimeDifference.Max, buttonTimeDifference);

					out.TargetTimeDifference.Min = std::min(out.TargetTimeDifference.Min, targetTimeDifference);
					out.TargetTimeDifference.Max = std::max(out.TargetTimeDifference.Max, targetTimeDifference);
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
		lastFrameAnyItemActive = thisFrameAnyItemActive;
		thisFrameAnyItemActive = Gui::IsAnyItemActive();

		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, Gui::GetStyle().WindowPadding);

		Gui::BeginChild("InputBaseChild", vec2(640.0f + 120.0f, 340.0f - 16.0f), true, ImGuiWindowFlags_None);
		{
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Input Script Path");

			Gui::PushItemDisabledAndTextColor();
			{
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				Gui::PathInputTextWithHint("##ScriptPath", "Script Path", &inScript.ScriptPath, ImGuiInputTextFlags_ReadOnly);
				Gui::PopItemWidth();
				Gui::Separator();

				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				Gui::PathInputTextWithHint("##SongPath", "Song Path", &inScript.SongPath, ImGuiInputTextFlags_ReadOnly);
				Gui::PopItemWidth();
				Gui::Separator();
			}
			Gui::PopItemDisabledAndTextColor();

			Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetColorU32(ImGuiCol_WindowBg));

			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("PV Script Source");
			Gui::NextColumn();
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Tempo Map Approximation");
			Gui::NextColumn();
			Gui::Separator();

			Gui::BeginChild("PVScriptSourceChild", vec2(Gui::GetContentRegionAvailWidth(), 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar | ImGuiWindowFlags_AlwaysHorizontalScrollbar);
			if (inScript.LoadedScript != nullptr)
			{
				SyntaxHighlightedPVCommandListViewGui(inScript.LoadedScript->Commands);
			}
			Gui::EndChild();

			Gui::NextColumn();

			Gui::BeginChild("TempoMapChild", vec2(Gui::GetContentRegionAvailWidth(), 0.0f), true, ImGuiWindowFlags_AlwaysVerticalScrollbar);
			if (!inScript.LoadedScript->Commands.empty() && outChart.ImportedChart != nullptr)
			{
				for (const auto& flyingTimeCommand : inScript.DecomposedScipt.FlyingTimeCommands)
				{
					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.TimeCommand);
					Gui::TextUnformatted(flyingTimeCommand.CommandTime.FormatTime().data());
					Gui::SameLine(0.0f, 0.0f);
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					Gui::TextUnformatted(": (");
					Gui::SameLine(0.0f, 0.0f);
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Parameters);
					Gui::Text("%.2f BPM", flyingTimeCommand.FlyingTempo.BeatsPerMinute);
					Gui::SameLine(0.0f, 0.0f);
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					Gui::TextUnformatted(", ");
					Gui::SameLine(0.0f, 0.0f);
					Gui::PopStyleColor();

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Parameters);
					Gui::Text("%d/%d", flyingTimeCommand.Signature.Numerator, flyingTimeCommand.Signature.Denominator);
					Gui::SameLine(0.0f, 0.0f);
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
			Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
			Gui::PushStyleColor(ImGuiCol_ChildBg, Gui::GetColorU32(ImGuiCol_WindowBg));

			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Import Settings");
			Gui::NextColumn();
			Gui::AlignTextToFramePadding();
			Gui::TextUnformatted("Import Statistics");
			Gui::NextColumn();
			Gui::Separator();

			Gui::BeginChild("SettingsChild", vec2(Gui::GetContentRegionAvailWidth(), 0.0f), true, ImGuiWindowFlags_None);
			{
				Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Target Offset");
				Gui::NextColumn();
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				if (f64 ms = importSettings.TargetOffset.TotalMilliseconds(); Gui::InputDouble("##TargetOffset", &ms, 1.0f, 10.0f, "%.4f ms"))
				{
					importSettings.TargetOffset = TimeSpan::FromMilliseconds(ms);
					UpdateImportedChartAndStatistics();
				}
				Gui::PopItemWidth();
				Gui::Separator();
				Gui::NextColumn();

				Gui::NextColumn();

				const bool setBeatAllignedDisabled = (importSettings.TargetOffset == inScript.FirstTargetBeatAlignmentOffset);
				Gui::PushItemDisabledAndTextColorIf(setBeatAllignedDisabled);
				if (Gui::Button("Beat-Align First Target", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
				{
					importSettings.TargetOffset = inScript.FirstTargetBeatAlignmentOffset;
					UpdateImportedChartAndStatistics();
				}
				Gui::PopItemDisabledAndTextColorIf(setBeatAllignedDisabled);
				Gui::NextColumn();
				Gui::Separator();

				Gui::AlignTextToFramePadding();
				Gui::TextUnformatted("Flying Time Factor");
				Gui::NextColumn();
				Gui::PushItemWidth(Gui::GetContentRegionAvailWidth());
				constexpr f32 minFlyingTimeFactor = 0.1f;
				if (importSettings.FlyingTimeFactor < minFlyingTimeFactor)
					importSettings.FlyingTimeFactor = 1.0f;
				if (Gui::InputFloat("##FlyingTimeFactor", &importSettings.FlyingTimeFactor, 0.1f, 1.0f, "%.2fx"))
				{
					importSettings.FlyingTimeFactor = std::max(minFlyingTimeFactor, importSettings.FlyingTimeFactor);
					UpdateImportedChartAndStatistics();
				}
				Gui::PopItemWidth();
				Gui::NextColumn();
				Gui::Separator();

				Gui::NextColumn();

				const bool setDefaultFlyingTimeDisabled = (importSettings.FlyingTimeFactor == 1.0f);
				Gui::PushItemDisabledAndTextColorIf(setDefaultFlyingTimeDisabled);
				if (Gui::Button("Set Default##FlyingTimeFactor", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
				{
					importSettings.FlyingTimeFactor = 1.0f;
					UpdateImportedChartAndStatistics();
				}
				Gui::PopItemDisabledAndTextColorIf(setDefaultFlyingTimeDisabled);
				Gui::NextColumn();
				Gui::Separator();

				Gui::EndColumns();
			}
			Gui::EndChild();

			Gui::NextColumn();

			Gui::BeginChild("StatisticsChild", vec2(Gui::GetContentRegionAvailWidth(), 0.0f), true, ImGuiWindowFlags_None);
			{
				{
					Gui::TextUnformatted("Target Time Precision Loss (Visual Only)");
					Gui::Separator();

					Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
					Gui::Text("Min, Max, Average");
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
					Gui::TextUnformatted("Button Time Precision Loss");
					Gui::Separator();

					Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
					Gui::Text("Min, Max, Average");
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
					Gui::TextUnformatted("Target Distribution");
					Gui::Separator();

					Gui::BeginColumns(nullptr, 2, ImGuiColumnsFlags_NoResize);
					for (size_t i = 0; i < importStatistics.BarDivisionsToCheckCount; i++)
					{
						const i32 barDivision = importStatistics.BarDivisionsToCheck[i];
						const size_t targetCount = importStatistics.BarDivisionDistribution[i];

						Gui::Text("Targets at 1 / %d", barDivision);
						Gui::NextColumn();

						Gui::Text("%zu", targetCount);
						Gui::NextColumn();

						Gui::Separator();
					}
					Gui::EndColumns();
				}
			}
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
				if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_YesOrOk, false))
					RequestExitAndImport();
				else if (Input::IsAnyPressed(GlobalUserData.Input.App_Dialog_Cancel, false))
					RequestExitWithoutImport();
			}

			if (Gui::Button("Import", vec2((Gui::GetContentRegionAvailWidth() - Gui::GetStyle().ItemSpacing.x) * 0.5f, Gui::GetContentRegionAvail().y)))
				RequestExitAndImport();
			Gui::SameLine();
			if (Gui::Button("Cancel", Gui::GetContentRegionAvail()))
				RequestExitWithoutImport();
		}
		Gui::EndChild();
		Gui::PopStyleColor();

		Gui::PopStyleVar();
	}

	void PVScriptImportWindow::SetScriptFilePath(std::string_view path)
	{
		inScript.ScriptPath = IO::Path::Normalize(path);
		inScript.SongPath = GetDefaultSongPathFromPVScriptPath(inScript.ScriptPath);

		inScript.LoadedScript = inScript.ScriptPath.empty() ? nullptr : IO::File::Load<PVScript>(inScript.ScriptPath);

		if (inScript.LoadedScript == nullptr)
			inScript.LoadedScript = std::make_unique<PVScript>();

		importSettings = {};
		inScript.DecomposedScipt = DecomposePVScriptChartData(*inScript.LoadedScript, path);

		UpdateImportedChartAndStatistics(false);

		inScript.FirstTargetBeatAlignmentOffset = CalculateFirstTargetBeatAlignmentTimeOffset(*outChart.ImportedChart);
		importSettings.TargetOffset = inScript.FirstTargetBeatAlignmentOffset;

		UpdateImportedChartAndStatistics();
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

		inScript = {};
		outChart = {};
		importSettings = {};
		importStatistics = {};

		return movedImportedChart;
	}

	void PVScriptImportWindow::UpdateImportedChartAndStatistics(bool updateStatistics)
	{
		outChart.ImportedChart = CreateImportChartFromDecomposedScript(inScript.DecomposedScipt, inScript.SongPath, importSettings);

		if (updateStatistics)
			importStatistics = CalculateImportStatistics(inScript.DecomposedScipt, *outChart.ImportedChart, importSettings);
	}

	void PVScriptImportWindow::RequestExitAndImport()
	{
		closeWindowThisFrame = true;
		// NOTE: The imported chart is expected to always be valid while the window is open
		// outChart.ImportedChart = ...;
	}

	void PVScriptImportWindow::RequestExitWithoutImport()
	{
		closeWindowThisFrame = true;
		outChart.ImportedChart = nullptr;
	}
}

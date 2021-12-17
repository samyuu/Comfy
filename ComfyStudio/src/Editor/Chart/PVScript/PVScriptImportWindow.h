#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Editor/Chart/Chart.h"
#include "Script/PVScript.h"
#include "PVScriptUtil.h"

namespace Comfy::Studio::Editor
{
	class PVScriptImportWindow : NonCopyable
	{
	public:
		struct ImportSettings
		{
			TimeSpan TargetOffset;
		};

		struct ImportStatistics
		{
			// NOTE: Count split into separate constexpr variable to avoid falsely detected intellisense warnings
			static constexpr size_t BarDivisionsToCheckCount = 8;
			static constexpr i32 BarDivisionsToCheck[] = { 8, 16, 24, 32, 48, 64, 96, 192 };

			struct MinMaxAverage { TimeSpan Min, Max, Average; };

			MinMaxAverage TargetTimeDifference;
			MinMaxAverage ButtonTimeDifference;

			std::array<size_t, BarDivisionsToCheckCount> BarDivisionDistribution;
		};

	public:
		PVScriptImportWindow() = default;
		~PVScriptImportWindow() = default;

	public:
		void Gui();

		void SetScriptFilePathAndStartAsyncLoading(std::string_view scriptPath);

		bool GetAndClearCloseRequestThisFrame();
		std::unique_ptr<Chart> TryMoveImportedChartBeforeClosing();

		// NOTE: Only access inScript.Async data if !IsAsyncLoading
		bool IsAsyncLoading() const;
		void WaitForAsyncLoadCompletion();

	private:
		void StartOfFrameCheckForAndHandleAsyncLoadCompletion();

		void CreateChartFromDecomposedScriptAssignToImportedChart();
		void RecalculateFirstTargetBeatAlignmentOffset();
		void RecalculateImportStatistics();

		void OnGuiImportSettingsValueChanged();
		void OnGuiTempoMapValueChanged();

		void RequestExitAndImport();
		void RequestExitWithoutImport();

	private:
		struct InputScriptData
		{
			// NOTE: Set inside main thread, safe to read from but shouldn't be modified while IsAsyncLoading()
			struct SyncData
			{
				std::string ScriptPath;
			} Sync;

			// NOTE: Set inside async worker thread, only safe to read from if !IsAsyncLoading()
			struct AsyncData
			{
				std::string SongPath;
				std::string MoviePath;
				std::unique_ptr<PVScript> LoadedScriptFile;
				DecomposedPVScriptChartData DecomposedScipt;
				DecomposedPVScriptChartData DecomposedSciptUneditedCopy;
				TimeSpan FirstTargetBeatAlignmentOffset;
			} Async;

			std::future<void> LoadAndDecomposeChartFileFuture;
		} inScript = {};

		struct OutputChartData
		{
			std::unique_ptr<Chart> ImportedChart;
		} outChart = {};

		std::string emptyDummyStringToReferenceWhileAsyncLoading;

		// NOTE: Potentially read from / written to on async worker thread but shouldn't really matter for POD data
		ImportSettings importSettings = {};
		ImportStatistics importStatistics = {};

		bool isFirstFrameAfterStartingAsyncLoad = false;
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;
	};
}

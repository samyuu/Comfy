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
			f32 FlyingTimeFactor;
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

		void SetScriptFilePath(std::string_view path);

		bool GetAndClearCloseRequestThisFrame();
		std::unique_ptr<Chart> TryMoveImportedChartBeforeClosing();

	private:
		void UpdateImportedChartAndStatistics(bool updateStatistics = true);

		void RequestExitAndImport();
		void RequestExitWithoutImport();

	private:
		struct InputScriptData
		{
			std::string ScriptPath, SongPath;
			std::unique_ptr<PVScript> LoadedScript;
			DecomposedPVScriptChartData DecomposedScipt;
			TimeSpan FirstTargetBeatAlignmentOffset;
		} inScript = {};

		struct OutputChartData
		{
			std::unique_ptr<Chart> ImportedChart;
		} outChart = {};

		ImportSettings importSettings = {};
		ImportStatistics importStatistics = {};

		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;
	};
}

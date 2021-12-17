#pragma once
#include "Types.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Script/PVScript.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	constexpr std::array<std::string_view, EnumCount<Difficulty>()> PVScriptFileNameDifficultyNames =
	{
		"easy",
		"normal",
		"hard",
		"extreme",
		"extreme_1",
	};

	struct DecompsedPVScriptFileName
	{
		i32 ID;
		Difficulty DifficultyType;
	};

	DecompsedPVScriptFileName DecompsePVScriptFileName(std::string_view fileNameWithoutExtension);

	constexpr f32 PVTargetCommandFixedPointPositionFactor = 250.0f;
	constexpr f32 PVTargetCommandFixedPointAngleFactor = 1000.0f;

	struct DecomposedPVScriptChartData
	{
		struct TargetCommandData
		{
			TimeSpan TargetTime;
			TimeSpan ButtonTime;
			PVCommandLayout::Target Parameters;
		};

		struct FlyingTimeCommandData
		{
			TimeSpan CommandTime;
			// NOTE: Flying time commands don't actually store data about the real tempo so this only exists here to allow easy editing in post
			Tempo RealTempo;
			Tempo FlyingTimeTempo;
			TimeSignature Signature;
		};

		std::string ScriptFileName;
		DecompsedPVScriptFileName DecomposedScriptName;

		std::optional<TimeSpan> MusicPlayCommandTime;
		std::optional<TimeSpan> MoviePlayCommandTime;
		TimeSpan PVEndCommandTime;
		std::vector<TargetCommandData> TargetCommands;
		std::vector<FlyingTimeCommandData> FlyingTimeCommands;
	};

	DecomposedPVScriptChartData DecomposePVScriptChartData(const PVScript& script, std::string_view scriptFilePath);

	struct SongAndMovieFilePathLists
	{
		std::vector<std::string> SongPaths;
		std::vector<std::string> MoviePaths;
	};

	SongAndMovieFilePathLists GetPotentialSongAndMovieFilePathsFromPVScriptPath(std::string_view scriptPath);

	constexpr bool IsChartRelatedPVCommand(PVCommandType commandType)
	{
		switch (commandType)
		{
		case PVCommandType::Target:
		case PVCommandType::MusicPlay:
		case PVCommandType::BarTimeSet:
		case PVCommandType::TargetFlyingTime:
			return true;
		case PVCommandType::MoviePlay:
		case PVCommandType::MovieDisp:
		case PVCommandType::MovieCutChg:
			return true;
		default:
			return false;
		}
	}

	constexpr struct PVCommandListViewSyntaxColorData
	{
		u32 TimeCommand = 0xFF44BB64;
		u32 BranchCommand = 0xFF3B3BDA;
		u32 TargetCommand = 0xFF348FDF;
		u32 ChartCommand = 0xFF31C3C2;
		u32 NeutralCommand = 0xFFDDDDDD;
		u32 EndCommand = 0xFFCA6531;
		u32 Indentation = 0xFF494949;
		u32 PlainText = 0xFFDCDCDC;
		u32 Parameters = 0xFF606060;
	} PVCommandListViewSyntaxColors;

	void SyntaxHighlightedPVCommandListViewGui(const std::vector<PVCommand>& commands);
}

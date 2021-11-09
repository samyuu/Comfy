#include "PVScriptUtil.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringUtil.h"
#include "Misc/StringParseHelper.h"

namespace Comfy::Studio::Editor
{
	DecompsedPVScriptFileName DecompsePVScriptFileName(std::string_view fileNameWithoutExtension)
	{
		if (!Util::StartsWithInsensitive(fileNameWithoutExtension, "pv_") || fileNameWithoutExtension.size() < std::string_view("pv_xxx_xxxx").size())
			return DecompsedPVScriptFileName { -1, Difficulty::Hard };

		const std::string_view idSubStr = fileNameWithoutExtension.substr(3, 3);
		const std::string_view difficultySubStr = fileNameWithoutExtension.substr(7);

		const i32 parsedID = Util::StringParsing::ParseType<i32>(idSubStr);

		const auto foundDifficultyIndex = FindIndexOf(PVScriptFileNameDifficultyNames, [difficultySubStr](auto& name) { return Util::MatchesInsensitive(name, difficultySubStr); });
		const auto foundDifficulty = InBounds(foundDifficultyIndex, PVScriptFileNameDifficultyNames) ? static_cast<Difficulty>(foundDifficultyIndex) : Difficulty::Hard;

		return DecompsedPVScriptFileName { parsedID, foundDifficulty };
	}

	DecomposedPVScriptChartData DecomposePVScriptChartData(const PVScript& script, std::string_view scriptFilePath)
	{
		DecomposedPVScriptChartData out = {};

		out.ScriptFileName = IO::Path::GetFileName(scriptFilePath, false);
		out.DecomposedScriptName = DecompsePVScriptFileName(out.ScriptFileName);

		auto isFlyingTimeSame = [](auto& a, auto& b) { return (a.FlyingTempo.BeatsPerMinute == b.FlyingTempo.BeatsPerMinute) && (a.Signature == b.Signature); };

		TimeSpan currentCmdTime = {};
		TimeSpan currentFlyingTime = TimeSpan::FromSeconds(1.0);

		for (const auto& cmd : script.Commands)
		{
			if (const auto* timeCmd = cmd.TryView<PVCommandLayout::Time>())
			{
				currentCmdTime = static_cast<TimeSpan>(*timeCmd);
			}
			else if (const auto* targetFlyingTimeCmd = cmd.TryView<PVCommandLayout::TargetFlyingTime>())
			{
				currentFlyingTime = static_cast<TimeSpan>(*targetFlyingTimeCmd);
				DecomposedPVScriptChartData::FlyingTimeCommandData newFlyingTime;
				newFlyingTime.CommandTime = currentCmdTime;
				newFlyingTime.FlyingTempo = glm::round((60.0f * 4.0f) / (static_cast<f32>(targetFlyingTimeCmd->DurationMS) / 1000.0f));
				newFlyingTime.Signature = TimeSignature(4, 4);

				if (out.FlyingTimeCommands.empty() || !isFlyingTimeSame(newFlyingTime, out.FlyingTimeCommands.back()))
					out.FlyingTimeCommands.push_back(newFlyingTime);
			}
			else if (const auto* barTimeSetCmd = cmd.TryView<PVCommandLayout::BarTimeSet>())
			{
				currentFlyingTime = static_cast<TimeSpan>(*barTimeSetCmd);
				DecomposedPVScriptChartData::FlyingTimeCommandData newFlyingTime;
				newFlyingTime.CommandTime = currentCmdTime;
				newFlyingTime.FlyingTempo = static_cast<f32>(barTimeSetCmd->BeatsPerMinute);
				newFlyingTime.Signature = TimeSignature(barTimeSetCmd->TimeSignature + 1, 4);

				if (out.FlyingTimeCommands.empty() || !isFlyingTimeSame(newFlyingTime, out.FlyingTimeCommands.back()))
					out.FlyingTimeCommands.push_back(newFlyingTime);
			}
			else if (const auto* targetCmd = cmd.TryView<PVCommandLayout::Target>())
			{
				auto& outTarget = out.TargetCommands.emplace_back();
				outTarget.TargetTime = currentCmdTime;
				outTarget.ButtonTime = currentCmdTime + currentFlyingTime;
				outTarget.Parameters = *targetCmd;
			}
			else if (const auto* musicPlayCmd = cmd.TryView<PVCommandLayout::MusicPlay>())
			{
				out.MusicPlayCommandTime = currentCmdTime;
			}
			else if (const auto* moviePlayCmd = cmd.TryView<PVCommandLayout::MoviePlay>())
			{
				out.MoviePlayCommandTime = currentCmdTime;
			}
			else if (const auto* pvEndCmd = cmd.TryView<PVCommandLayout::PVEnd>())
			{
				out.PVEndCommandTime = currentCmdTime;
			}
		}

		return out;
	}

	std::string GetDefaultSongPathFromPVScriptPath(std::string_view scriptPath)
	{
		if (scriptPath.empty())
			return "";

		const std::string_view fileNameNoExtension = IO::Path::GetFileName(scriptPath, false);
		const std::string_view scriptDirectory = IO::Path::GetDirectoryName(scriptPath);
		const std::string_view romDirectory = IO::Path::GetDirectoryName(scriptDirectory);

		if (fileNameNoExtension.empty() || scriptDirectory.empty() || romDirectory.empty())
			return "";

		std::string songPath = IO::Path::Normalize(romDirectory);
		songPath += "/sound/song/";
		songPath += fileNameNoExtension.substr(0, Min(fileNameNoExtension.size(), std::string_view("pv_xxx").size()));
		songPath += ".ogg";
		return songPath;
	}

	void SyntaxHighlightedPVCommandListViewGui(const std::vector<PVCommand>& commands)
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(1.0f));

		ImGuiListClipper clipper; clipper.Begin(static_cast<i32>(commands.size()));
		while (clipper.Step())
		{
			for (i32 i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			{
				const auto& command = commands[i];

				char buffer[64];
				auto guiSameLineText = [](std::string_view text) { Gui::TextUnformatted(Gui::StringViewStart(text), Gui::StringViewEnd(text)); Gui::SameLine(0.0f, 0.0f); };

				i32 levelsOfIndentation = 0;
				u32 commandTypeColor = PVCommandListViewSyntaxColors.NeutralCommand;

				if (command.Type == PVCommandType::Time)
				{
					levelsOfIndentation = 1;
					commandTypeColor = PVCommandListViewSyntaxColors.TimeCommand;
				}
				else if (command.Type == PVCommandType::PVBranchMode)
				{
					levelsOfIndentation = 0;
					commandTypeColor = PVCommandListViewSyntaxColors.BranchCommand;
				}
				else
				{
					levelsOfIndentation = 2;

					if (command.Type == PVCommandType::Target)
						commandTypeColor = PVCommandListViewSyntaxColors.TargetCommand;
					else if (IsChartRelatedPVCommand(command.Type))
						commandTypeColor = PVCommandListViewSyntaxColors.ChartCommand;
					else if (command.Type == PVCommandType::End || command.Type == PVCommandType::PVEnd || command.Type == PVCommandType::PVEndFadeout)
						commandTypeColor = PVCommandListViewSyntaxColors.EndCommand;
				}

				Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Indentation);
				static constexpr std::array<std::string_view, 3> indentationStrings = { "", "|\t", "|\t|\t" };
				guiSameLineText(indentationStrings[levelsOfIndentation]);
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, commandTypeColor);
				guiSameLineText(command.Name().data());
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
				guiSameLineText("(");
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.Parameters);
				if (command.Type == PVCommandType::Time)
				{
					guiSameLineText(TimeSpan(command.View<PVCommandLayout::Time>()).FormatTime().data());
				}
				else if (command.Type == PVCommandType::ModeSelect)
				{
					const u32 difficultyFlags = command.View<PVCommandLayout::ModeSelect>().DifficultyFlags;
					if (difficultyFlags <= (1 << 7))
					{
						buffer[0] = '0';
						buffer[1] = 'b';
						char* bufferPostPrefix = &buffer[2];
						for (u32 i = 0; i < 8; i++)
							bufferPostPrefix[7 - i] = (difficultyFlags & (1 << i)) ? '1' : '0';
						bufferPostPrefix[8] = '\0';
					}
					else
					{
						sprintf_s(buffer, "0x%X", difficultyFlags);
					}

					guiSameLineText(buffer);

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					guiSameLineText(", ");
					Gui::PopStyleColor();

					sprintf_s(buffer, "%d", command.View<PVCommandLayout::ModeSelect>().ModeType);
					guiSameLineText(buffer);
				}
				else if (command.Type == PVCommandType::BarTimeSet)
				{
					sprintf_s(buffer, "%d BPM", command.View<PVCommandLayout::BarTimeSet>().BeatsPerMinute);
					guiSameLineText(buffer);

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
					guiSameLineText(", ");
					Gui::PopStyleColor();

					sprintf_s(buffer, "%d/4", command.View<PVCommandLayout::BarTimeSet>().TimeSignature + 1);
					guiSameLineText(buffer);
				}
				else if (command.Type == PVCommandType::TargetFlyingTime)
				{
					guiSameLineText(TimeSpan(command.View<PVCommandLayout::TargetFlyingTime>()).FormatTime().data());
				}
				else if (command.Type == PVCommandType::PVBranchMode)
				{
					static constexpr auto branchNames = std::array { "Default", "Failure", "Success" };
					const auto branchMode = command.View<PVCommandLayout::PVBranchMode>().Mode;

					Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.BranchCommand);
					guiSameLineText(IndexOr(static_cast<i32>(branchMode), branchNames, "Unknown"));
					Gui::PopStyleColor();
				}
				else
				{
					const u32 paramCount = command.ParamCount();
					for (size_t i = 0; i < paramCount; i++)
					{
						char intStrBuffer[34];
						::_itoa_s(static_cast<i32>(command.Param[i]), intStrBuffer, 10);

						guiSameLineText(intStrBuffer);

						if (i + 1 != paramCount)
						{
							Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
							guiSameLineText(", ");
							Gui::PopStyleColor();
						}
					}
				}
				Gui::PopStyleColor();

				Gui::PushStyleColor(ImGuiCol_Text, PVCommandListViewSyntaxColors.PlainText);
				Gui::TextUnformatted(");");
				Gui::PopStyleColor();

				Gui::Separator();
			}
		}

		Gui::PopStyleVar();
	}
}

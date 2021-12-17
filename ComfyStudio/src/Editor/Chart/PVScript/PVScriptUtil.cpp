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

		auto isFlyingTimeSame = [](auto& a, auto& b) { return (a.FlyingTimeTempo.BeatsPerMinute == b.FlyingTimeTempo.BeatsPerMinute) && (a.Signature == b.Signature); };

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
				newFlyingTime.FlyingTimeTempo = glm::round((60.0f * 4.0f) / (static_cast<f32>(targetFlyingTimeCmd->DurationMS) / 1000.0f));
				newFlyingTime.RealTempo = newFlyingTime.FlyingTimeTempo;
				newFlyingTime.Signature = TimeSignature(4, 4);

				if (out.FlyingTimeCommands.empty() || !isFlyingTimeSame(newFlyingTime, out.FlyingTimeCommands.back()))
					out.FlyingTimeCommands.push_back(newFlyingTime);
			}
			else if (const auto* barTimeSetCmd = cmd.TryView<PVCommandLayout::BarTimeSet>())
			{
				currentFlyingTime = static_cast<TimeSpan>(*barTimeSetCmd);
				DecomposedPVScriptChartData::FlyingTimeCommandData newFlyingTime;
				newFlyingTime.CommandTime = currentCmdTime;
				newFlyingTime.FlyingTimeTempo = static_cast<f32>(barTimeSetCmd->BeatsPerMinute);
				newFlyingTime.RealTempo = newFlyingTime.FlyingTimeTempo;
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
				// NOTE: Only account for the first play command
				if (!out.MusicPlayCommandTime.has_value())
					out.MusicPlayCommandTime = currentCmdTime;
			}
			else if (const auto* moviePlayCmd = cmd.TryView<PVCommandLayout::MoviePlay>())
			{
				if (!out.MoviePlayCommandTime.has_value())
					out.MoviePlayCommandTime = currentCmdTime;
			}
			else if (const auto* pvEndCmd = cmd.TryView<PVCommandLayout::PVEnd>())
			{
				out.PVEndCommandTime = currentCmdTime;
			}
		}

		return out;
	}

	SongAndMovieFilePathLists GetPotentialSongAndMovieFilePathsFromPVScriptPath(std::string_view scriptPath)
	{
		SongAndMovieFilePathLists out = {};
		if (scriptPath.empty())
			return out;

		const std::string normalizedScriptPath { IO::Path::Normalize(scriptPath) };
		const std::string scriptNameNoExtension { IO::Path::GetFileName(normalizedScriptPath, false) };
		const std::string scriptDirectory { IO::Path::GetDirectoryName(normalizedScriptPath) };
		const std::string romDirectory { IO::Path::GetDirectoryName(scriptDirectory) };
		const std::string maybeMDataIDDirectory { IO::Path::GetDirectoryName(romDirectory) };
		const std::string maybeMDataDirectory { IO::Path::GetDirectoryName(maybeMDataIDDirectory) };
		const std::string maybeRootDirectory { IO::Path::GetDirectoryName(maybeMDataDirectory) };
		const std::string maybeRootRomDirectory { maybeRootDirectory + "/rom" };
		const std::string pvIDFileNameNoExtension { scriptNameNoExtension.substr(0, Min(scriptNameNoExtension.size(), std::string_view("pv_xxx").size())) };

		out.SongPaths =
		{
			romDirectory + "/sound/song/" + pvIDFileNameNoExtension + ".ogg",
			romDirectory + "/sound/song/" + scriptNameNoExtension + ".ogg",
			romDirectory + "/" + pvIDFileNameNoExtension + ".ogg",
			romDirectory + "/" + scriptNameNoExtension + ".ogg",
			maybeRootRomDirectory + "/sound/song/" + pvIDFileNameNoExtension + ".ogg",
			maybeRootRomDirectory + "/sound/song/" + scriptNameNoExtension + ".ogg",
			maybeRootRomDirectory + "/" + pvIDFileNameNoExtension + ".ogg",
			maybeRootRomDirectory + "/" + scriptNameNoExtension + ".ogg",
		};
		out.MoviePaths =
		{
			romDirectory + "/movie/" + pvIDFileNameNoExtension + ".mp4",
			romDirectory + "/movie/" + scriptNameNoExtension + ".mp4",
			romDirectory + "/" + pvIDFileNameNoExtension + ".mp4",
			romDirectory + "/" + scriptNameNoExtension + ".mp4",
			romDirectory + "/movie/" + pvIDFileNameNoExtension + ".wmv",
			romDirectory + "/movie/" + scriptNameNoExtension + ".wmv",
			romDirectory + "/" + pvIDFileNameNoExtension + ".wmv",
			romDirectory + "/" + scriptNameNoExtension + ".wmv",
			maybeRootRomDirectory + "/movie/" + pvIDFileNameNoExtension + ".mp4",
			maybeRootRomDirectory + "/movie/" + scriptNameNoExtension + ".mp4",
			maybeRootRomDirectory + "/" + pvIDFileNameNoExtension + ".mp4",
			maybeRootRomDirectory + "/" + scriptNameNoExtension + ".mp4",
			maybeRootRomDirectory + "/movie/" + pvIDFileNameNoExtension + ".wmv",
			maybeRootRomDirectory + "/movie/" + scriptNameNoExtension + ".wmv",
			maybeRootRomDirectory + "/" + pvIDFileNameNoExtension + ".wmv",
			maybeRootRomDirectory + "/" + scriptNameNoExtension + ".wmv",
		};

		assert(out.SongPaths.capacity() == out.SongPaths.size());
		assert(out.MoviePaths.capacity() == out.MoviePaths.size());
		return out;
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

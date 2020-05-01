#include "CommandLineOption.h"
#include "Core/Logger.h"
#include "System/Version/BuildVersion.h"
#include "System/Version/BuildConfiguration.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "IO/Archive/FArc.h"
#include "IO/Directory.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringHelper.h"

namespace Comfy::System
{
	static void FArcProcessor(int index, const char* arguments[])
	{
		const auto farcPath = std::string_view(arguments[index]);

		if (auto farc = IO::FArc::Open(farcPath); farc)
		{
			std::string directory;
			directory.append(IO::Path::GetDirectoryName(farcPath));
			directory.append("\\");
			directory.append(IO::Path::GetFileName(farcPath, false));

			IO::Directory::Create(directory);

			for (const auto& entry : farc->GetEntries())
			{
				auto data = entry.ReadArray();
				IO::File::WriteAllBytes(directory + "\\" + entry.Name, data.get(), entry.OriginalSize);
			}
		}
	}

	static void AetSetFormatProcessor(int index, const char* arguments[])
	{
		const auto inputPath = arguments[index + 0];
		const auto outputPath = arguments[index + 1];

		if (auto aetSet = IO::File::Load<Graphics::Aet::AetSet>(inputPath); aetSet != nullptr)
			IO::File::Save(outputPath, *aetSet);
	}

	const char* CommandLineOption::GetDescription() const
	{
		return (Description != nullptr) ? Description : "No Description";
	}

	bool CommandLineOption::MatchesCommand(const char* input) const
	{
		return MatchesCommand(input, ShortCommand, LongCommand);
	}

	bool CommandLineOption::MatchesCommand(const char* input, const char* shortCommand, const char* longCommand)
	{
		return std::strcmp(input, shortCommand) == 0 || std::strcmp(input, longCommand) == 0;
	}

	std::vector<CommandLineOption> CommandLineOptions::options;

	void CommandLineOptions::Initialize()
	{
		options =
		{
			{ "-v",		"--version",		"Print comfy version info",			true,	0, [](int index, const char* arguments[]) { Logger::LogLine("Comfy Studio - Version: %s (%s) - %s", BuildVersion::CommitHash, BuildVersion::CommitNumber, BuildVersion::CommitTime); } },
			{ "-cfg",	"--build_config",	"Print comfy build configuration",	true,	0, [](int index, const char* arguments[]) { Logger::LogLine("Comfy Build Config: %s", BuildConfiguration::Debug ? "Debug" : BuildConfiguration::Release ? "Release" : "Unknown"); } },
			{ "-t",		"--test",			"Print a test message",				true,	0, [](int index, const char* arguments[]) { Logger::LogLine(">> Test Message"); } },
			{ "-te",	"--test_echo",		"Echo the following argument",		false,	1, [](int index, const char* arguments[]) { Logger::LogLine(">> echo '%s'", arguments[index]); } },
			{ "-e",		"--exit",			"Exit the application",				true,	0, [](int index, const char* arguments[]) {} },
			{ "-f",		"--farc",			"Extract FArc",						true,	1, FArcProcessor },
			{ "-aet",	"--aet_reformat",	"Reformat an AetSet",				true,	2, AetSetFormatProcessor },
		};
	}

	const std::vector<CommandLineOption>& CommandLineOptions::GetOptions()
	{
		return options;
	}
}

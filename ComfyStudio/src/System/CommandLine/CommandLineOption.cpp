#include "CommandLineOption.h"
#include "Core/Logger.h"
#include "System/Version/BuildVersion.h"
#include "System/Version/BuildConfiguration.h"
#include "FileSystem/Archive/Farc.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace System
{
	static void FarcProcessor(int index, const char* arguments[])
	{
		using namespace FileSystem;

		const std::string filePath = arguments[index];
		RefPtr<Farc> farc = Farc::Open(filePath);
		if (farc)
		{
			const std::string baseDirectory = GetDirectory(filePath);
			const std::string directory = baseDirectory + "\\" + GetFileName(filePath, false);
			CreateDirectoryFile(Utf8ToUtf16(directory));

			for (const ArchiveEntry& entry : *farc)
			{
				std::vector<uint8_t> data = entry.ReadVector();
				WriteAllBytes(directory + std::string("\\") + entry.Name, data);
			}
		}
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
		return strcmp(input, shortCommand) == 0 || strcmp(input, longCommand) == 0;
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
			{ "-f",		"--farc",			"Extract Farc",						true,	1, FarcProcessor },
		};
	}

	const std::vector<CommandLineOption>& CommandLineOptions::GetOptions()
	{
		return options;
	}
}
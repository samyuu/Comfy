#include "CommandLineOption.h"
#include "Core/Logger.h"
#include <cstring>

namespace System
{
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
			{ "-t", "--test", "Test Message", false, 0, [](int index, const char* arguments[]) { Logger::LogLine(">> Test Response"); } },
			{ "-p", "--print", "Print next Message", false, 1, [](int index, const char* arguments[]) { Logger::LogLine(">> Print(%s)", arguments[index]); } },
			{ "-v", "--version", "Print version Message", false, 0, [](int index, const char* arguments[]) { Logger::LogLine(">> Test Version 1.0"); } },
			{ "-e", "--exit", "Exit Application", true, 0, [](int index, const char* arguments[]) {} },
		};
	}

	const std::vector<CommandLineOption>& CommandLineOptions::GetOptions()
	{
		return options;
	}
}
#include "CommandLine.h"
#include "Core/Logger.h"

namespace Comfy::Studio::CLI
{
	CommandLineResult CommandLine::Parse(int count, const char* arguments[], bool ignoreInvalid)
	{
		if (count < 2)
			return CommandLineResult::Continue;

		CommandLineOptions::Initialize();
		const char* programName = arguments[0];

		int currentArgumentIndex = 1;
		int remainingArgumentCount = count - currentArgumentIndex;

		if (CommandLineOption::MatchesCommand(arguments[currentArgumentIndex], "-h", "--help"))
		{
			if (remainingArgumentCount == 2)
			{
				// NOTE: Print help for next command
				for (const CommandLineOption& option : CommandLineOptions::GetOptions())
				{
					if (option.MatchesCommand(arguments[currentArgumentIndex + 1]))
						Logger::LogLine("%s, %s : %s", option.ShortCommand, option.LongCommand, option.GetDescription());
				}
			}
			else
			{
				// NOTE: Print help for all available commands
				for (const CommandLineOption& option : CommandLineOptions::GetOptions())
					Logger::LogLine("%s, %s : %s", option.ShortCommand, option.LongCommand, option.GetDescription());
			}

			return CommandLineResult::Exit;
		}

		bool anyExits = false;
		while (remainingArgumentCount > 0)
		{
			CommandProcessResult commandResult = ProcessCommand(arguments, currentArgumentIndex, remainingArgumentCount);

			if (commandResult == CommandProcessResult::Exit)
			{
				anyExits = true;
			}
			else if (commandResult == CommandProcessResult::Invalid)
			{
				if (ignoreInvalid)
					return CommandLineResult::Continue;

				OnInvalidArgument(arguments[currentArgumentIndex]);
				return CommandLineResult::Exit;
			}
		}

		if (anyExits)
			return CommandLineResult::Exit;

		return CommandLineResult::Continue;
	}

	CommandProcessResult CommandLine::ProcessCommand(const char* arguments[], int& currentArgumentIndex, int& remainingArgumentCount)
	{
		for (const CommandLineOption& option : CommandLineOptions::GetOptions())
		{
			const char* currentArgument = arguments[currentArgumentIndex];

			if (remainingArgumentCount < 1)
				break;

			if (!option.MatchesCommand(currentArgument))
				continue;

			currentArgumentIndex++;
			remainingArgumentCount--;

			if (option.ArgumentCount > remainingArgumentCount)
			{
				OnInsufficientArguments(option);
				return CommandProcessResult::Exit;
			}

			option.Response(currentArgumentIndex, arguments);
			currentArgumentIndex += option.ArgumentCount;
			remainingArgumentCount -= option.ArgumentCount;

			if (option.ExitExecution)
				return CommandProcessResult::Exit;

			return CommandProcessResult::Valid;
		}

		return CommandProcessResult::Invalid;
	}

	void CommandLine::OnInsufficientArguments(const CommandLineOption& option)
	{
		Logger::LogErrorLine("Insufficient number of arguments for %s", option.LongCommand);
	}

	void CommandLine::OnInvalidArgument(const char* input)
	{
		bool isCommand = input[0] == '-';
		Logger::LogErrorLine(isCommand ? "Unrecognized option '%s'" : "Invalid argument '%s'", input);
	}
}

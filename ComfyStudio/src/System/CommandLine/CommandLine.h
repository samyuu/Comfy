#pragma once
#include "CommandLineOption.h"

namespace Comfy::System
{
	enum class CommandLineResult
	{
		Continue, Exit
	};

	enum class CommandProcessResult
	{
		Valid, Invalid, Exit
	};

	class CommandLine
	{
		CommandLine() = delete;

	public:
		static CommandLineResult Parse(int count, const char* arguments[]);

	private:
		static CommandProcessResult ProcessCommand(const char* arguments[], int& currentArgumentIndex, int& remainingArgumentCount);
		static void OnInsufficientArguments(const CommandLineOption& option);
		static void OnInvalidArgument(const char* input);
	};
}

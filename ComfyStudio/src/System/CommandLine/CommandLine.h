#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "CommandLineOption.h"

namespace Comfy::Studio::System
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
		static CommandLineResult Parse(int count, const char* arguments[], bool ignoreInvalid = true);

	private:
		static CommandProcessResult ProcessCommand(const char* arguments[], int& currentArgumentIndex, int& remainingArgumentCount);
		static void OnInsufficientArguments(const CommandLineOption& option);
		static void OnInvalidArgument(const char* input);
	};
}

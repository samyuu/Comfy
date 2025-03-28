#pragma once
#include "Types.h"
#include <functional>

namespace Comfy::Studio::CLI
{
	struct CommandLineOption
	{
		const char* ShortCommand;
		const char* LongCommand;
		const char* Description;

		bool ExitExecution;
		int ArgumentCount;
		std::function<void(int index, const char* arguments[])> Response;

		const char* GetDescription() const;
		bool MatchesCommand(const char* input) const;
		static bool MatchesCommand(const char* input, const char* shortCommand, const char* longCommand);
	};

	class CommandLineOptions
	{
		CommandLineOptions() = delete;
	
	public:
		static void Initialize();
		static const std::vector<CommandLineOption>& GetOptions();
	
	private:
		static std::vector<CommandLineOption> options;
	};
}

#define COMFY_RUN_MAIN_TEST 0

#if (COMFY_DEBUG)
#define COMFY_USE_WIN_MAIN 0
#else
#define COMFY_USE_WIN_MAIN 1
#endif /* COMFY_DEBUG */

#if (COMFY_RUN_MAIN_TEST)
#include "MainTest.cpp"

int main(int argc, const char* argv[])
{
	return MainTest();
}
#else

#include "Core/Application.h"
#include "System/CommandLine/CommandLine.h"
#include "IO/Path.h"
#include "IO/Directory.h"
#include "Core/Win32/ComfyWindows.h"
#include <shellapi.h>

namespace Comfy::Studio
{
	struct CommandLineView
	{
		int Count;
		const char** Arguments;
	};

	CommandLineView GetCommandLineUTF8()
	{
		static std::vector<std::string> argvString;
		static std::vector<const char*> argvCStr;

		assert(argvString.empty() && argvCStr.empty());

		int argc = 0;
		auto argv = ::CommandLineToArgvW(::GetCommandLineW(), &argc);

		argvString.reserve(argc);
		argvCStr.reserve(argc);

		for (auto i = 0; i < argc; i++)
			argvCStr.emplace_back(argvString.emplace_back(UTF8::Narrow(argv[i])).c_str());

		::LocalFree(argv);
		return { argc, argvCStr.data() };
	}

	int Main()
	{
		auto[argc, argv] = GetCommandLineUTF8();
		const auto commandLineResult = System::CommandLine::Parse(argc, argv);

		if (commandLineResult == System::CommandLineResult::Exit)
			return EXIT_SUCCESS;

		const auto fileToOpen = (argc == 2) ? std::string_view(argv[1]) : "";
		if (!fileToOpen.empty())
		{
			// HACK: This is to correctly handle relative paths and avoid "leaving behind" config files when using Comfy Studio to open files with
			//		 while still allowing a working directory to be set explicitly for (graphics-) debugging
			const auto executableDirectory = IO::Path::GetDirectoryName(argv[0]);
			IO::Directory::SetWorkingDirectory(executableDirectory);
		}

		auto application = Application(fileToOpen);
		application.Run();

		return EXIT_SUCCESS;
	}
}

#if (COMFY_USE_WIN_MAIN)
int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	return Comfy::Studio::Main();
}
#else
int main(int argc, const char* argv[])
{
	return Comfy::Studio::Main();
}
#endif /* COMFY_USE_WIN_MAIN */

#endif /* COMFY_RUN_MAIN_TEST */

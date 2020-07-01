#include "Core/Application.h"
#include "System/CommandLine/CommandLine.h"

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

#if (COMFY_USE_WIN_MAIN)

int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	const auto argc = (__argc);
	const auto argv = const_cast<const char**>(__argv);

#else

int main(int argc, const char* argv[])
{

#endif /* COMFY_USE_WIN_MAIN */

	const auto commandLineResult = Comfy::Studio::System::CommandLine::Parse(argc, argv);

	if (commandLineResult == Comfy::Studio::System::CommandLineResult::Exit)
		return EXIT_SUCCESS;

	Comfy::Studio::Application application;
	application.Run();

	return EXIT_SUCCESS;
}
#endif /* COMFY_RUN_MAIN_TEST */

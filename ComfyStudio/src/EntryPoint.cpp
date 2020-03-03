#include "Core/Application.h"
#include "System/CommandLine/CommandLine.h"

#define RUN_MAIN_TEST 0
#define USE_WIN_MAIN 1

#if (RUN_MAIN_TEST)
#include "MainTest.cpp"

int main(int argc, const char* argv[])
{
	return MainTest();
}
#else

#if (USE_WIN_MAIN)

int WinMain(HINSTANCE instance, HINSTANCE previousInstance, LPSTR commandLine, int showCommand)
{
	const auto argc = (__argc);
	const auto argv = const_cast<const char**>(__argv);

#else

int main(int argc, const char* argv[])
{

#endif /* USE_WIN_MAIN */

	const auto commandLineResult = Comfy::System::CommandLine::Parse(argc, argv);

	if (commandLineResult == Comfy::System::CommandLineResult::Exit)
		return EXIT_SUCCESS;

	Comfy::Application application;
	application.Run();

	return EXIT_SUCCESS;
}
#endif /* RUN_MAIN_TEST */

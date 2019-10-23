#include "Core/Application.h"
#include "System/CommandLine/CommandLine.h"

#define RUN_MAIN_TEST 0

#if (RUN_MAIN_TEST)
#include "MainTest.cpp"

int main(int argc, const char* argv[])
{
	return MainTest();
}
#else
int main(int argc, const char* argv[])
{
	System::CommandLineResult commandLineResult = System::CommandLine::Parse(argc, argv);

	if (commandLineResult == System::CommandLineResult::Exit)
		return EXIT_SUCCESS;

	Application application;
	application.Run();

	return EXIT_SUCCESS;
}
#endif
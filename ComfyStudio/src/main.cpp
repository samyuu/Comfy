#include "pch.h"
#include "Core/Application.h"
#include "System/CommandLine/CommandLine.h"

#define RUN_MAIN_TEST 0
int MainTest();

int main(int argc, const char* argv[])
{
#if (RUN_MAIN_TEST)
	return MainTest();
#endif

	System::CommandLineResult commandLineResult = System::CommandLine::Parse(argc, argv);

	if (commandLineResult == System::CommandLineResult::Exit)
		return EXIT_SUCCESS;

	Application* application = new Application();
	application->Run();

	delete application;
	application = nullptr;

	return EXIT_SUCCESS;
}
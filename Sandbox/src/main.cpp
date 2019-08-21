#include "pch.h"
#include "Core/Application.h"

void MainTest();

int main()
{
	//MainTest(); return 0;

	UniquePtr<Application> application = MakeUnique<Application>();
	application->Run();
}
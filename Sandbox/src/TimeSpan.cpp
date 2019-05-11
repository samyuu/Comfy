#include "TimeSpan.h"
#include "glfw/glfw3.h"

TimeSpan TimeSpan::GetTimeNow()
{
	return glfwGetTime();
}

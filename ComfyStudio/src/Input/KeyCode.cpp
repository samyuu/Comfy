#include "KeyCode.h"
#include <glfw/glfw3.h>

const char* GetKeyCodeName(KeyCode keyCode)
{
	return glfwGetKeyName(keyCode, glfwGetKeyScancode(keyCode));
}

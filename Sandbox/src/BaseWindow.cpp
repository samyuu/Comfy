#include "BaseWindow.h"

BaseWindow::BaseWindow(Application* parent) : parentApplication(parent)
{
}

BaseWindow::~BaseWindow()
{
}

ImGuiWindowFlags BaseWindow::GetWindowFlags()
{
	return ImGuiWindowFlags_None;
}

#include "BaseWindow.h"

BaseWindow::BaseWindow(Application* parent) : parentApplication(parent)
{
}

BaseWindow::~BaseWindow()
{
}

ImGuiWindowFlags BaseWindow::GetWindowFlags() const
{
	return ImGuiWindowFlags_None;
}

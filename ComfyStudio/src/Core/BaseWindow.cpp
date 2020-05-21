#include "BaseWindow.h"

namespace Comfy::Studio
{
	BaseWindow::BaseWindow(Application* parent) : parentApplication(parent)
	{
	}

	ImGuiWindowFlags BaseWindow::GetWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}
}

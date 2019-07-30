#include "Engine.h"

namespace App
{
	Engine::Engine()
	{
	}

	Engine::~Engine()
	{
	}

	void Engine::Tick()
	{
		guiWindow = ImGui::GetCurrentWindow();

		ImGui::Text(__FUNCTION__ "(): Test");
	}
}
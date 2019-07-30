#pragma once
#include "ImGui/imgui_extensions.h"

namespace App
{
	class Engine
	{
	public:
		Engine();
		Engine(const Engine& other) = delete;
		~Engine();

		void Tick();

	protected:
		ImGuiWindow* guiWindow;
	};
}
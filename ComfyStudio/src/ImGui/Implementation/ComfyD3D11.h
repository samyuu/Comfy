#pragma once
#include "ImGui/Core/imgui.h"

namespace ImGui
{
	namespace ComfyD3D11
	{
		bool Initialize();
		void Shutdown();

		bool CreateDeviceObjects();
		void InvalidateDeviceObjects();

		void NewFrame();
		void RenderDrawData(const ImDrawData* drawData);
	}
}

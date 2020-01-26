#pragma once
#include "ImGui/Gui.h"
#include "Graphics/Camera.h"

namespace Editor
{
	void DrawCameraAxisIndicationGui(ImDrawList* drawList, const Graphics::PerspectiveCamera& camera, vec2 indicatorCenter, float indicatorSize, float indicatorPadding, vec2 textOffset);
}

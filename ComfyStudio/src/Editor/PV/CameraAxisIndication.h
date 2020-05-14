#pragma once
#include "ImGui/Gui.h"
#include "Render/core/Camera.h"

namespace Comfy::Editor
{
	void DrawCameraAxisIndicationGui(ImDrawList* drawList, const Render::PerspectiveCamera& camera, vec2 indicatorCenter, float indicatorSize, float indicatorPadding, vec2 textOffset);
}

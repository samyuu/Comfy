#pragma once
#include "Types.h"
#include "Render/Render.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	void DrawCameraAxisIndicationGui(ImDrawList* drawList, const Render::PerspectiveCamera& camera, vec2 indicatorCenter, float indicatorSize, float indicatorPadding, vec2 textOffset);
}

#pragma once
#include "Types.h"
#include "ImGui/Core/imgui.h"

namespace ImGui
{
	constexpr vec2 PopupWindowPadding = vec2(6.0f, 4.0f);

	void StyleComfy(ImGuiStyle* dst = nullptr);
}
#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Graphics/TexSet.h"

namespace ImGui
{
	class CheckerboardTexture : NonCopyable
	{
	public:
		CheckerboardTexture(vec4 color = vec4(0.15f, 0.15f, 0.15f, 1.0f), vec4 colorAlt = vec4(0.32f, 0.32f, 0.32f, 1.0f), int gridSize = 4);
		~CheckerboardTexture() = default;

	public:
		void AddToDrawList(ImDrawList* drawList, ImRect region);

	private:
		Comfy::Graphics::Tex texture;
	};
}

#pragma once
#include "ImGui/Core/imgui.h"
#include "Graphics/TexSet.h"

namespace ImGui
{
	// NOTE: For textures that originate from 2D sprites
	void ImageSprTex(const Comfy::Graphics::Tex* tex, const ImVec2& size = { 0.0f, 0.0f });
	
	// NOTE: For textures that originate from object materials
	void ImageObjTex(const Comfy::Graphics::Tex* tex, const ImVec2& size = { 0.0f, 0.0f });
}

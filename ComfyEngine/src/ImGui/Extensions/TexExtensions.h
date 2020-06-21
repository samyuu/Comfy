#pragma once
#include "ImGui/Core/imgui.h"
#include "Graphics/TexSet.h"
#include "Graphics/Auth2D/SprSet.h"
#include <optional>

namespace ImGui
{
	// NOTE: For textures that originate from 2D sprites
	void ImageSprTex(const Comfy::Graphics::Tex* tex, std::optional<vec2> size = {});
	void ImageSprTex(const Comfy::Graphics::Tex* tex, const Comfy::Graphics::Spr* spr, std::optional<vec2> size = {});
	
	// NOTE: For textures that originate from object materials
	void ImageObjTex(const Comfy::Graphics::Tex* tex, std::optional<vec2> size = {});
}

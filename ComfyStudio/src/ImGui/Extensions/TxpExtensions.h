#pragma once
#include "ImGui/Core/imgui.h"
#include "Graphics/TxpSet.h"

namespace ImGui
{
	// NOTE: For txps that originate from 2D sprites
	void ImageSprTxp(const Graphics::Txp* txp, const ImVec2& size = { 0.0f, 0.0f });
	
	// NOTE: For txps that originate from object materials
	void ImageObjTxp(const Graphics::Txp* txp, const ImVec2& size = { 0.0f, 0.0f });
}

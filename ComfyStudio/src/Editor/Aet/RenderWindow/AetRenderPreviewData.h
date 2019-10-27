#pragma once
#include "Graphics/Auth2D/AetSet.h"

namespace Editor
{
	// NOTE: To communicate and provide visual feedback between different components
	struct AetRenderPreviewData
	{
		const Graphics::AetRegion* AetRegion = nullptr;
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Unknown;
	};
}
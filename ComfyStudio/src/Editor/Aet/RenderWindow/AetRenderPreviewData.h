#pragma once
#include "Graphics/Auth2D/AetSet.h"

namespace Comfy::Editor
{
	// NOTE: To communicate and provide visual feedback between different components
	struct AetRenderPreviewData
	{
		const Graphics::AetSurface* Surface = nullptr;
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Unknown;
	};
}

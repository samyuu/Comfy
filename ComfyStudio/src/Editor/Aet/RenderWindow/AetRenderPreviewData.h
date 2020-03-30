#pragma once
#include "Graphics/Auth2D/Aet/AetSet.h"

namespace Comfy::Editor
{
	// NOTE: To communicate and provide visual feedback between different components
	struct AetRenderPreviewData
	{
		const Graphics::Aet::Video* Video = nullptr;
		Graphics::AetBlendMode BlendMode = Graphics::AetBlendMode::Unknown;
	};
}

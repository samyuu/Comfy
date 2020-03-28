#pragma once
#include "Graphics/D3D11/State/BlendState.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics::D3D11
{
	struct BlendStateCache
	{
	public:
		BlendStateCache();
		BlendState& GetState(BlendFactor source, BlendFactor destination);

	private:
		std::array<std::array<UniquePtr<BlendState>, static_cast<size_t>(BlendFactor::Count)>, static_cast<size_t>(BlendFactor::Count)> states;
	};
}

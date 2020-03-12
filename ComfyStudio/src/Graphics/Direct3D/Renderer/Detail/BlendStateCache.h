#pragma once
#include "Graphics/Direct3D/State/D3D_BlendState.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics
{
	struct BlendStateCache
	{
	public:
		BlendStateCache();
		D3D_BlendState& GetState(BlendFactor source, BlendFactor destination);

	private:
		std::array<std::array<UniquePtr<D3D_BlendState>, static_cast<size_t>(BlendFactor::Count)>, static_cast<size_t>(BlendFactor::Count)> states;
	};
}

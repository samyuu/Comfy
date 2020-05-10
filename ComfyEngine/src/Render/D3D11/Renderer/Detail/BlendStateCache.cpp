#include "BlendStateCache.h"

namespace Comfy::Render::D3D11
{
	namespace
	{
		constexpr std::array D3DBlendFactors = { D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_DEST_COLOR, };
	}

	BlendStateCache::BlendStateCache()
	{
		for (int src = 0; src < static_cast<int>(BlendFactor::Count); src++)
		{
			for (int dst = 0; dst < static_cast<int>(BlendFactor::Count); dst++)
			{
				states[src][dst] = std::make_unique<BlendState>(D3DBlendFactors[src], D3DBlendFactors[dst], D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE);
				D3D11_SetObjectDebugName(states[src][dst]->GetBlendState(), "Renderer3D::BlendState::%s-%s", BlendFactorNames[src], BlendFactorNames[dst]);
			}
		}
	}

	BlendState& BlendStateCache::GetState(BlendFactor source, BlendFactor destination)
	{
		return *states[static_cast<size_t>(source)][static_cast<size_t>(destination)];
	}
}

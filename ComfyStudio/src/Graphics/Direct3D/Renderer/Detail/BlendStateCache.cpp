#include "BlendStateCache.h"

namespace Comfy::Graphics
{
	namespace
	{
		constexpr std::array D3DBlendFactors = { D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_INV_SRC_COLOR, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_DEST_COLOR, };
		constexpr std::array BlendFactorNames = { "Zero", "One", "SrcColor", "ISrcColor", "SrcAlpha", "ISrcAlpha", "DstAlpha", "IDstAlpha", "DstColor", "IDstColor", };
	}

	BlendStateCache::BlendStateCache()
	{
		for (int src = 0; src < BlendFactor_Count; src++)
		{
			for (int dst = 0; dst < BlendFactor_Count; dst++)
			{
				states[src][dst] = MakeUnique<D3D_BlendState>(D3DBlendFactors[src], D3DBlendFactors[dst], D3D11_BLEND_INV_DEST_ALPHA, D3D11_BLEND_ONE);
				D3D_SetObjectDebugName(states[src][dst]->GetBlendState(), "Renderer3D::BlendState::%s-%s", BlendFactorNames[src], BlendFactorNames[dst]);
			}
		}
	}

	D3D_BlendState& BlendStateCache::GetState(BlendFactor source, BlendFactor destination)
	{
		return *states[source][destination];
	}
}

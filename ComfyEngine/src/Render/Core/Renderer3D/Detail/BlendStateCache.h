#pragma once
#include "Graphics/Auth3D/ObjSet.h"
#include "Render/D3D11/State/BlendState.h"

namespace Comfy::Render::Detail
{
	static constexpr size_t BlendFactorCount = EnumCount<Graphics::BlendFactor>();

	constexpr std::array D3DBlendFactors = 
	{ 
		D3D11_BLEND_ZERO, 
		D3D11_BLEND_ONE, 
		D3D11_BLEND_SRC_COLOR, 
		D3D11_BLEND_INV_SRC_COLOR, 
		D3D11_BLEND_SRC_ALPHA, 
		D3D11_BLEND_INV_SRC_ALPHA, 
		D3D11_BLEND_DEST_ALPHA, 
		D3D11_BLEND_INV_DEST_ALPHA, 
		D3D11_BLEND_DEST_COLOR, 
		D3D11_BLEND_INV_DEST_COLOR, 
	};

	struct BlendStateCache
	{
	public:
		BlendStateCache()
		{
			for (size_t src = 0; src < BlendFactorCount; src++)
			{
				for (size_t dst = 0; dst < BlendFactorCount; dst++)
				{
					states[src][dst] = std::make_unique<D3D11::BlendState>(
						D3DBlendFactors[src], 
						D3DBlendFactors[dst], 
						D3D11_BLEND_INV_DEST_ALPHA, 
						D3D11_BLEND_ONE);
					
					D3D11_SetObjectDebugName(states[src][dst]->GetBlendState(), 
						"Renderer3D::BlendState::%s-%s", Graphics::BlendFactorNames[src], Graphics::BlendFactorNames[dst]);
				}
			}
		}

		D3D11::BlendState& GetState(Graphics::BlendFactor source, Graphics::BlendFactor destination)
		{
			return *states[static_cast<size_t>(source)][static_cast<size_t>(destination)];
		}

	private:
		std::array<std::array<std::unique_ptr<D3D11::BlendState>, BlendFactorCount>, BlendFactorCount> states;
	};
}

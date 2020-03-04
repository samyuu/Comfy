#include "TextureSamplerCache.h"

namespace Comfy::Graphics
{
	namespace
	{
		constexpr std::array D3DAddressModes = { D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP };
		constexpr std::array AddressModeNames = { "Mirror", "Repeat", "Clamp" };
	}

	void TextureSamplerCache::CreateIfNeeded(const RenderParameters& renderParameters)
	{
		if (samplers[0][0] == nullptr || lastAnistropicFiltering != renderParameters.AnistropicFiltering)
		{
			const auto filter = (renderParameters.AnistropicFiltering > D3D11_MIN_MAXANISOTROPY) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;

			for (int u = 0; u < AddressMode_Count; u++)
			{
				for (int v = 0; v < AddressMode_Count; v++)
				{
					samplers[u][v] = MakeUnique<D3D_TextureSampler>(filter, D3DAddressModes[u], D3DAddressModes[v], 0.0f, renderParameters.AnistropicFiltering);
					D3D_SetObjectDebugName(samplers[u][v]->GetSampler(), "Renderer3D::Sampler::%s-%s", AddressModeNames[u], AddressModeNames[v]);
				}
			}

			lastAnistropicFiltering = renderParameters.AnistropicFiltering;
		}
	}

	D3D_TextureSampler& TextureSamplerCache::GetSampler(MaterialTextureFlags flags)
	{
		auto u = flags.TextureAddressMode_U_Mirror ? Mirror : flags.TextureAddressMode_U_Repeat ? Repeat : Clamp;
		auto v = flags.TextureAddressMode_V_Mirror ? Mirror : flags.TextureAddressMode_V_Repeat ? Repeat : Clamp;
		return *samplers[u][v];
	}
}

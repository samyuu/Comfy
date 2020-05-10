#include "TextureSamplerCache.h"

namespace Comfy::Graphics::D3D11
{
	namespace
	{
		constexpr std::array D3DAddressModes = { D3D11_TEXTURE_ADDRESS_MIRROR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_TEXTURE_ADDRESS_CLAMP };
		constexpr std::array AddressModeNames = { "Mirror", "Repeat", "Clamp" };
	}

	void TextureSamplerCache::CreateIfNeeded(const SceneRenderParameters& renderParameters)
	{
		if (samplers[0][0] == nullptr || lastAnistropicFiltering != renderParameters.AnistropicFiltering)
		{
			const auto filter = (renderParameters.AnistropicFiltering > D3D11_MIN_MAXANISOTROPY) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;

			for (int u = 0; u < AddressMode_Count; u++)
			{
				for (int v = 0; v < AddressMode_Count; v++)
				{
					samplers[u][v] = std::make_unique<TextureSampler>(filter, D3DAddressModes[u], D3DAddressModes[v], 0.0f, renderParameters.AnistropicFiltering);
					D3D11_SetObjectDebugName(samplers[u][v]->GetSampler(), "Renderer3D::Sampler::%s-%s", AddressModeNames[u], AddressModeNames[v]);
				}
			}

			lastAnistropicFiltering = renderParameters.AnistropicFiltering;
		}
	}

	TextureSampler& TextureSamplerCache::GetSampler(MaterialTextureData::TextureSamplerFlags flags)
	{
		const auto u = flags.MirrorU ? Mirror : flags.RepeatU ? Repeat : Clamp;
		const auto v = flags.MirrorV ? Mirror : flags.RepeatV ? Repeat : Clamp;
		return *samplers[u][v];
	}
}

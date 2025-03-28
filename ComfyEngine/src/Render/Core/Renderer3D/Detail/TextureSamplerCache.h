#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"
#include "Render/D3D11/D3D11Texture.h"

namespace Comfy::Render::Detail
{
	constexpr std::array D3DAddressModes =
	{
		D3D11_TEXTURE_ADDRESS_MIRROR,
		D3D11_TEXTURE_ADDRESS_WRAP,
		D3D11_TEXTURE_ADDRESS_CLAMP,
	};

	constexpr std::array AddressModeNames =
	{
		"Mirror",
		"Repeat",
		"Clamp",
	};

	struct TextureSamplerCache3D
	{
	public:
		void CreateIfNeeded(i32 anistropicFiltering)
		{
			if (!samplers[0][0].has_value() || lastAnistropicFiltering != anistropicFiltering)
			{
				const auto filter = (anistropicFiltering > D3D11_MIN_MAXANISOTROPY) ? D3D11_FILTER_ANISOTROPIC : D3D11_FILTER_MIN_MAG_MIP_LINEAR;

				for (int u = 0; u < AddressMode_Count; u++)
				{
					for (int v = 0; v < AddressMode_Count; v++)
					{
						samplers[u][v].emplace(GlobalD3D11, filter, D3DAddressModes[u], D3DAddressModes[v], 0.0f, anistropicFiltering);
						D3D11_SetObjectDebugName(samplers[u][v]->SamplerState.Get(), "Renderer3D::Sampler::%s-%s", AddressModeNames[u], AddressModeNames[v]);
					}
				}

				lastAnistropicFiltering = anistropicFiltering;
			}
		}

		D3D11TextureSampler& GetSampler(Graphics::MaterialTextureData::TextureSamplerFlags flags)
		{
			const auto u = flags.MirrorU ? Mirror : flags.RepeatU ? Repeat : Clamp;
			const auto v = flags.MirrorV ? Mirror : flags.RepeatV ? Repeat : Clamp;
			return *samplers[u][v];
		}

	private:
		enum AddressMode { Mirror, Repeat, Clamp, AddressMode_Count };

		i32 lastAnistropicFiltering = -1;
		std::array<std::array<std::optional<D3D11TextureSampler>, AddressMode_Count>, AddressMode_Count> samplers;
	};
}

#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "Render/Core/Renderer2D/RenderCommand2D.h"
#include "Render/D3D11/D3D11Texture.h"
#include "Render/D3D11/D3D11GraphicsTypeHelpers.h"
#include <optional>

namespace Comfy::Render::Detail
{
	struct TextureSamplerCache2D
	{
	public:
		D3D11TextureSampler& GetSampler(Render::TexSamplerView texSamplerView)
		{
			using namespace Graphics;
			assert(texSamplerView.AddressU < TextureAddressMode::Count && texSamplerView.AddressV < TextureAddressMode::Count);
			assert(texSamplerView.Filter < TextureFilter::Count);

			auto& uvArray = samplers[static_cast<size_t>(texSamplerView.Filter)];
			auto& vArray = uvArray[static_cast<size_t>(texSamplerView.AddressV)];
			auto& sampler = vArray[static_cast<size_t>(texSamplerView.AddressU)];

			if (!sampler.has_value())
			{
				sampler.emplace(
					GlobalD3D11,
					TextureFilterToD3D(texSamplerView.Filter),
					TextureAddressModeToD3D(texSamplerView.AddressU),
					TextureAddressModeToD3D(texSamplerView.AddressV));

				D3D11_SetObjectDebugName(sampler->SamplerState.Get(), "Renderer2D::Sampler::%s-%s::%s",
					TextureAddressModeNames[static_cast<size_t>(texSamplerView.AddressU)],
					TextureAddressModeNames[static_cast<size_t>(texSamplerView.AddressV)],
					TextureFilterNames[static_cast<size_t>(texSamplerView.Filter)]);
			}

			return sampler.value();
		}

	private:
		using WrapUSamplerArray = std::array<std::optional<D3D11TextureSampler>, EnumCount<Graphics::TextureAddressMode>()>;
		using WrapUVSamplerArray = std::array<WrapUSamplerArray, EnumCount<Graphics::TextureAddressMode>()>;
		using WrapUVFilterSamplerArray = std::array<WrapUVSamplerArray, EnumCount<Graphics::TextureFilter>()>;

		WrapUVFilterSamplerArray samplers;
	};
}

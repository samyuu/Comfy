#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "Render/Core/Renderer2D/RenderCommand2D.h"
#include "Render/D3D11/Texture/TextureSampler.h"
#include <optional>

namespace Comfy::Render::Detail
{
	struct TextureSamplerCache2D
	{
	public:
		D3D11::TextureSampler& GetSampler(Render::TexSamplerView texSamplerView)
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
					D3D11::TextureFilterToD3D(texSamplerView.Filter),
					D3D11::TextureAddressModeToD3D(texSamplerView.AddressU),
					D3D11::TextureAddressModeToD3D(texSamplerView.AddressV));

				D3D11_SetObjectDebugName(sampler->GetSampler(), "Renderer2D::Sampler::%s-%s::%s",
					TextureAddressModeNames[static_cast<size_t>(texSamplerView.AddressU)],
					TextureAddressModeNames[static_cast<size_t>(texSamplerView.AddressV)],
					TextureFilterNames[static_cast<size_t>(texSamplerView.Filter)]);
			}

			return sampler.value();
		}

	private:
		using WrapUSamplerArray = std::array<std::optional<D3D11::TextureSampler>, EnumCount<Graphics::TextureAddressMode>()>;
		using WrapUVSamplerArray = std::array<WrapUSamplerArray, EnumCount<Graphics::TextureAddressMode>()>;
		using WrapUVFilterSamplerArray = std::array<WrapUVSamplerArray, EnumCount<Graphics::TextureFilter>()>;

		WrapUVFilterSamplerArray samplers;
	};
}

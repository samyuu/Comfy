#pragma once
#include "../Direct3D.h"
#include "../GraphicsInterfaces.h"

namespace Comfy::Graphics
{
	class D3D_TextureSampler : IGraphicsResource
	{
	public:
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV);
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV);
		D3D_TextureSampler(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering);
		virtual ~D3D_TextureSampler() = default;
		
	public:
		virtual void Bind(uint32_t samplerSlot) const;
		virtual void UnBind() const;

	public:
		template <size_t Size>
		static void BindArray(uint32_t startSlot, const std::array<D3D_TextureSampler*, Size>& samplers);

	public:
		ID3D11SamplerState* GetSampler();
		const D3D11_SAMPLER_DESC& GetDescription() const;

	protected:
		mutable uint32_t lastBoundSlot;
		D3D11_SAMPLER_DESC samplerDescription;

		ComPtr<ID3D11SamplerState> samplerState;
	};

	template<size_t Size>
	inline void D3D_TextureSampler::BindArray(uint32_t startSlot, const std::array<D3D_TextureSampler*, Size>& samplers)
	{
		std::array<ID3D11SamplerState*, Size> samplerStates;

		for (size_t i = 0; i < Size; i++)
			samplerStates[i] = (samplers[i] != nullptr) ? samplers[i]->GetSampler() : nullptr;

		D3D.Context->PSSetSamplers(startSlot, static_cast<UINT>(samplerStates.size()), samplerStates.data());
	}
}

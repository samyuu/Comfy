#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "Graphics/TxpSet.h"

namespace Graphics
{
	class D3D_Texture2D : IGraphicsResource
	{
	protected:
		D3D_Texture2D(D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode, float lodBias);
		virtual ~D3D_Texture2D() = default;

	public:
		virtual void Bind(uint32_t textureSlot) const;
		virtual void UnBind() const;
		
	public:
		ivec2 GetSize() const;

		ID3D11Texture2D* GetTexture();
		TextureFormat GetTextureFormat() const;

		void* GetVoidTexture() const;

	protected:
		mutable uint32_t lastBoundSlot;
		TextureFormat textureFormat;

		D3D11_SAMPLER_DESC samplerDescription;
		D3D11_TEXTURE2D_DESC textureDescription;

		ComPtr<ID3D11SamplerState> samplerState;
		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> resourceView;
	};

	class D3D_ImmutableTexture2D final : public D3D_Texture2D
	{
	public:
		D3D_ImmutableTexture2D(Txp* txp);
		D3D_ImmutableTexture2D(ivec2 size, const void* rgbaBuffer);
		D3D_ImmutableTexture2D(ivec2 size, const void* rgbaBuffer, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);
		D3D_ImmutableTexture2D(const D3D_ImmutableTexture2D&) = delete;
		~D3D_ImmutableTexture2D() = default;

		D3D_ImmutableTexture2D& operator=(const D3D_ImmutableTexture2D&) = delete;
	
	private:
	};

	/*
	class D3D_DynamicTexture2D final : public D3D_Texture2D
	{
	};
	*/
}

#pragma once
#include "Direct3D.h"
#include "D3D_TextureSampler.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	class D3D_TextureResource : IGraphicsResource
	{
	protected:
		D3D_TextureResource();
		virtual ~D3D_TextureResource() = default;

	public:
		void Bind(uint32_t textureSlot) const;
		void UnBind() const;

	public:
		virtual uint32_t GetArraySize() const = 0;

		ivec2 GetSize() const;
		TextureFormat GetTextureFormat() const;

		ID3D11Texture2D* GetTexture();
		ID3D11ShaderResourceView* GetResourceView() const;

	protected:
		mutable uint32_t lastBoundSlot;
		TextureFormat textureFormat;
		
		D3D11_TEXTURE2D_DESC textureDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> resourceView;
	};

	class D3D_Texture2D final : public D3D_TextureResource
	{
	public:
		D3D_Texture2D(const struct Txp& txp);
		D3D_Texture2D(ivec2 size, const void* rgbaBuffer);
		D3D_Texture2D(const D3D_Texture2D&) = delete;
		~D3D_Texture2D() = default;

		D3D_Texture2D& operator=(const D3D_Texture2D&) = delete;

	public:
		uint32_t GetArraySize() const override;

	private:
	};

	class D3D_CubeMap final : public D3D_TextureResource 
	{
	public:
		D3D_CubeMap(const struct Txp& txp);
		D3D_CubeMap(const D3D_CubeMap&) = delete;
		~D3D_CubeMap() = default;

		D3D_CubeMap& operator=(const D3D_CubeMap&) = delete;

	public:
		uint32_t GetArraySize() const override;

	private:
	};
}

#pragma once
#include "Direct3D.h"
#include "D3D_TextureSampler.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	class D3D_ShaderResourceView
	{
	public:
		virtual ID3D11ShaderResourceView* GetResourceView() const = 0;

	public:
		template <size_t Size>
		static void BindArray(uint32_t startSlot, const std::array<D3D_ShaderResourceView*, Size>& resources);
	};

	class D3D_TextureResource : public D3D_ShaderResourceView, IGraphicsResource
	{
	protected:
		D3D_TextureResource();
		virtual ~D3D_TextureResource();

	public:
		void Bind(uint32_t textureSlot) const;
		void UnBind() const;

	public:
		virtual uint32_t GetArraySize() const = 0;

		ivec2 GetSize() const;
		TextureFormat GetTextureFormat() const;

		ID3D11Texture2D* GetTexture();

		ID3D11ShaderResourceView* GetResourceView() const override;

	protected:
		mutable uint32_t lastBoundSlot;
		TextureFormat textureFormat;

		D3D11_TEXTURE2D_DESC textureDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> resourceView;
	};

	class D3D_Texture1D final : public D3D_ShaderResourceView
	{
	public:
		D3D_Texture1D(int32_t width, const void* pixelData, DXGI_FORMAT format);
		D3D_Texture1D(const D3D_Texture1D&) = delete;
		~D3D_Texture1D() = default;

		D3D_Texture1D& operator=(const D3D_Texture1D&) = delete;

	public:
		void UploadData(size_t dataSize, const void* pixelData);

	public:
		ID3D11ShaderResourceView* GetResourceView() const override;

	private:
		D3D11_TEXTURE1D_DESC textureDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;

		ComPtr<ID3D11Texture1D> texture;
		ComPtr<ID3D11ShaderResourceView> resourceView;
	};

	class D3D_Texture2D final : public D3D_TextureResource
	{
	public:
		static constexpr ivec2 MinSize = { 1, 1 };
		static constexpr ivec2 MaxSize = { D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION };

	public:
		D3D_Texture2D(const struct Txp& txp);
		D3D_Texture2D(ivec2 size, const uint32_t* rgbaBuffer);
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
		D3D_CubeMap(const struct LightMap& lightMap);
		D3D_CubeMap(const D3D_CubeMap&) = delete;
		~D3D_CubeMap() = default;

		D3D_CubeMap& operator=(const D3D_CubeMap&) = delete;

	public:
		uint32_t GetArraySize() const override;

	private:
	};

	template<size_t Size>
	inline void D3D_ShaderResourceView::BindArray(uint32_t startSlot, const std::array<D3D_ShaderResourceView*, Size>& resources)
	{
		std::array<ID3D11ShaderResourceView*, Size> resourceViews;

		for (size_t i = 0; i < Size; i++)
			resourceViews[i] = (resources[i] != nullptr) ? resources[i]->GetResourceView() : nullptr;

		D3D.Context->PSSetShaderResources(startSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}
}

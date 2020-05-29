#pragma once
#include "Types.h"
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"
#include "TextureSampler.h"

namespace Comfy::Graphics
{
	class Tex;
	struct LightMapIBL;
}

namespace Comfy::Render::D3D11
{
	class ShaderResourceView
	{
	public:
		virtual ID3D11ShaderResourceView* GetResourceView() const = 0;

	public:
		template <size_t Size>
		static void BindArray(u32 startSlot, const std::array<const ShaderResourceView*, Size>& resources);
	};

	class TextureResource : public ShaderResourceView, public IGraphicsResource
	{
	protected:
		TextureResource();
		virtual ~TextureResource();

	public:
		void Bind(u32 textureSlot) const;
		void UnBind() const;

	public:
		virtual u32 GetArraySize() const = 0;

		ivec2 GetSize() const;
		Graphics::TextureFormat GetTextureFormat() const;

		ID3D11Texture2D* GetTexture();

		ID3D11ShaderResourceView* GetResourceView() const override;

	protected:
		mutable u32 lastBoundSlot;
		Graphics::TextureFormat textureFormat;

		D3D11_TEXTURE2D_DESC textureDescription;
		D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDescription;

		ComPtr<ID3D11Texture2D> texture;
		ComPtr<ID3D11ShaderResourceView> resourceView;
	};

	class Texture1D final : public ShaderResourceView, NonCopyable
	{
	public:
		Texture1D(i32 width, const void* pixelData, DXGI_FORMAT format);
		~Texture1D() = default;

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

	class Texture2D final : public TextureResource
	{
	public:
		static constexpr ivec2 MinSize = { 1, 1 };
		static constexpr ivec2 MaxSize = { D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION };

	public:
		Texture2D(const Graphics::Tex& tex);
		Texture2D(ivec2 size, const u32* rgbaBuffer);
		~Texture2D() = default;

	public:
		u32 GetArraySize() const override;

	private:
	};

	class CubeMap final : public TextureResource
	{
	public:
		CubeMap(const Graphics::Tex& tex);
		CubeMap(const Graphics::LightMapIBL& lightMap);
		~CubeMap() = default;

	public:
		u32 GetArraySize() const override;

	private:
	};

	template<size_t Size>
	void ShaderResourceView::BindArray(u32 startSlot, const std::array<const ShaderResourceView*, Size>& resources)
	{
		std::array<ID3D11ShaderResourceView*, Size> resourceViews;

		for (size_t i = 0; i < Size; i++)
			resourceViews[i] = (resources[i] != nullptr) ? resources[i]->GetResourceView() : nullptr;

		D3D.Context->PSSetShaderResources(startSlot, static_cast<UINT>(resourceViews.size()), resourceViews.data());
	}
}

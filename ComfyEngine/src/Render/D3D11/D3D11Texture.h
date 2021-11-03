#pragma once
#include "Types.h"
#include "D3D11.h"
#include "D3D11OpaqueResource.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render
{
	struct D3D11RenderTargetAndView;

	struct D3D11Texture1DAndView : NonCopyable
	{
		D3D11Texture1DAndView(D3D11& d3d11, i32 width, const void* pixelData, DXGI_FORMAT format, D3D11_USAGE usage);
		~D3D11Texture1DAndView() = default;

		void UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* pixelData);

		D3D11_TEXTURE1D_DESC TextureDesc = {};
		ComPtr<ID3D11Texture1D> Texture = {};
		ComPtr<ID3D11ShaderResourceView> TextureView = {};
	};

	constexpr ivec2 D3D11MinTexture2DSize = { 1, 1 };
	constexpr ivec2 D3D11MaxTexture2DSize = { D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION, D3D11_REQ_TEXTURE2D_U_OR_V_DIMENSION };

	// NOTE: This includes CubeMaps which are represented as 6 sided 2D textures
	struct D3D11Texture2DAndView : public D3D11OpaqueResource, NonCopyable
	{
		D3D11Texture2DAndView(D3D11& d3d11, const Graphics::Tex& tex, D3D11_USAGE usage);
		D3D11Texture2DAndView(D3D11& d3d11, const Graphics::LightMapIBL& lightMap);
		D3D11Texture2DAndView(D3D11& d3d11, ivec2 size, const u32* rgbaBuffer, D3D11_USAGE usage);
		D3D11Texture2DAndView(D3D11& d3d11, const D3D11RenderTargetAndView& sourceRenderTargetToCopy);
		~D3D11Texture2DAndView() override;

		void Bind(D3D11& d3d11, u32 textureSlot) const;
		void UnBind(D3D11& d3d11) const;
		void UploadDataIfDynamic(D3D11& d3d11, const Graphics::Tex& tex);
		void CreateCopyFrom(D3D11& d3d11, const D3D11RenderTargetAndView& sourceRenderTargetToCopy);
		ivec2 GetSize() const;
		bool GetIsDynamic() const;
		bool GetIsCubeMap() const;

		D3D11& D3D11RefForDeferedDeletion;
		mutable u32 LastBoundSlot = {};
		Graphics::TextureFormat TextureFormat = {};
		D3D11_TEXTURE2D_DESC TextureDesc = {};
		ComPtr<ID3D11Texture2D> Texture = {};
		ComPtr<ID3D11ShaderResourceView> TextureView = {};
	};

	struct D3D11TextureSampler : NonCopyable
	{
		D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeUV);
		D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV);
		D3D11TextureSampler(D3D11& d3d11, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressModeU, D3D11_TEXTURE_ADDRESS_MODE addressModeV, float mipMapBias, int anisotropicFiltering);
		~D3D11TextureSampler() = default;

		void Bind(D3D11& d3d11, u32 samplerSlot) const;
		void UnBind(D3D11& d3d11) const;

		mutable u32 LastBoundSlot = {};
		D3D11_SAMPLER_DESC SamplerDesc = {};
		ComPtr<ID3D11SamplerState> SamplerState = {};
	};

	constexpr ivec2 D3D11RenderTargetDefaultSize = D3D11MinTexture2DSize;

	constexpr DXGI_FORMAT D3D11RenderTargetHDRFormatRGBA = DXGI_FORMAT_R16G16B16A16_FLOAT;
	constexpr DXGI_FORMAT D3D11RenderTargetLDRFormatRGBA = DXGI_FORMAT_R8G8B8A8_UNORM;

	struct D3D11RenderTargetAndView : NonCopyable
	{
		D3D11RenderTargetAndView(D3D11& d3d11, ivec2 size, DXGI_FORMAT colorFormat, u32 multiSampleCount = 1);
		D3D11RenderTargetAndView(D3D11& d3d11, ivec2 size, DXGI_FORMAT colorFormat, DXGI_FORMAT depthBufferFormat, u32 multiSampleCount = 1);
		~D3D11RenderTargetAndView() = default;

		void Bind(D3D11& d3d11) const;
		void BindAndSetViewport(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;
		
		void BindColorTexturePS(D3D11& d3d11, u32 textureSlot) const;
		void BindDepthTexturePS(D3D11& d3d11, u32 textureSlot) const;
		void UnBindPS(D3D11& d3d11, u32 textureSlot) const;

		void ClearColor(D3D11& d3d11, vec4 color);
		void ClearDepth(D3D11& d3d11, f32 depth = 1.0f);
		void ClearColorAndDepth(D3D11& d3d11, vec4 color, f32 depth = 1.0f);

		ivec2 GetSize() const;
		void RecreateWithNewSizeIfDifferent(D3D11& d3d11, ivec2 newSize);
		void RecreateWithNewColorFormatIfDifferent(D3D11& d3d11, DXGI_FORMAT newColorFormat);
		
		u32 GetMultiSampleCount() const;
		void RecreateWithNewMultiSampleCountIfDifferent(D3D11& d3d11, u32 newMultiSampleCount);
		
		bool GetHasColorBuffer() const;
		bool GetHasDepthBuffer() const;

		std::unique_ptr<u8[]> CopyColorPixelsBackToCPU(D3D11& d3d11);

		mutable u32 LastBoundSlot = {};
		
		// NOTE: Either the ColorTexture, the DepthTexture or BOTH can be valid at a time
		D3D11_TEXTURE2D_DESC ColorTextureDesc = {};
		ComPtr<ID3D11Texture2D> ColorTexture = {};
		ComPtr<ID3D11ShaderResourceView> ColorTextureView = {};
		ComPtr<ID3D11RenderTargetView> RenderTargetView = {};

		D3D11_TEXTURE2D_DESC DepthTextureDesc = {};
		ComPtr<ID3D11Texture2D> DepthTexture = {};
		ComPtr<ID3D11ShaderResourceView> DepthTextureView = {};
		ComPtr<ID3D11DepthStencilView> DepthStencilView = {};
	};
}

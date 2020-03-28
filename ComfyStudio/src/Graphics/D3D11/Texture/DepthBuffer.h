#pragma once
#include "../Direct3D.h"
#include "Texture.h"

namespace Comfy::Graphics::D3D11
{
	class DepthBuffer : IGraphicsResource
	{
	protected:
		DepthBuffer(ivec2 size, DXGI_FORMAT textureFormat, DXGI_FORMAT depthFormat, D3D11_BIND_FLAG bindFlags, uint32_t multiSampleCount);

	public:
		DepthBuffer(ivec2 size, DXGI_FORMAT format, uint32_t multiSampleCount = 1);
		~DepthBuffer() = default;

	public:
		void Clear(float value = 1.0f);
		virtual void Resize(ivec2 newSize);

		void SetMultiSampleCount(uint32_t multiSampleCount);

	public:
		uint32_t GetMultiSampleCount() const;
		ivec2 GetSize() const;

		ID3D11DepthStencilView* GetDepthStencilView() const;

	protected:
		D3D11_TEXTURE2D_DESC textureDescription;
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDescription;

		ComPtr<ID3D11Texture2D> depthTexture;
		ComPtr<ID3D11DepthStencilView> depthStencilView;
	};

	class ResourceViewDepthBuffer final : public ShaderResourceView, public DepthBuffer
	{
	public:
		ResourceViewDepthBuffer(ivec2 size, DXGI_FORMAT format);
		~ResourceViewDepthBuffer() = default;

	public:
		void Resize(ivec2 newSize) override;

		ID3D11ShaderResourceView* GetResourceView() const override;

	private:
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDescription;
		ComPtr<ID3D11ShaderResourceView> shaderResourceView;
	};
}

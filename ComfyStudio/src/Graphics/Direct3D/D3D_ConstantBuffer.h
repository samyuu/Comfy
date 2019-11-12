#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_ConstantBuffer : IGraphicsResource
	{
	protected:
		D3D_ConstantBuffer(uint32_t slot, size_t dataSize, D3D11_USAGE usage);
		virtual ~D3D_ConstantBuffer() = default;

	public:
		virtual void BindVertexShader();
		virtual void UnBindVertexShader();

		virtual void BindPixelShader();
		virtual void UnBindPixelShader();

	protected:
		uint32_t slot;

		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	class D3D_DynamicConstantBuffer final : public D3D_ConstantBuffer
	{
	public:
		D3D_DynamicConstantBuffer(uint32_t slot, size_t dataSize);
		D3D_DynamicConstantBuffer(const D3D_DynamicConstantBuffer&) = delete;
		~D3D_DynamicConstantBuffer() = default;

		D3D_DynamicConstantBuffer& operator=(const D3D_DynamicConstantBuffer&) = delete;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

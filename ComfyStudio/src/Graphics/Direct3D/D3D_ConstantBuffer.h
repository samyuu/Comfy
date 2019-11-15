#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_ConstantBuffer : IGraphicsResource
	{
	protected:
		D3D_ConstantBuffer(uint32_t slot, size_t dataSize, D3D11_USAGE usage, UINT accessFlags);
		virtual ~D3D_ConstantBuffer() = default;

	public:
		// TODO: Static methods to bind multiple buffers at once and reduce API overhead
		virtual void BindVertexShader();
		virtual void UnBindVertexShader();

		virtual void BindPixelShader();
		virtual void UnBindPixelShader();

	public:
		ID3D11Buffer* GetBuffer();

	protected:
		uint32_t slot;

		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	// NOTE: Use for data that changes less than once per frame
	class D3D_DefaultConstantBuffer final : public D3D_ConstantBuffer
	{
	public:
		D3D_DefaultConstantBuffer(uint32_t slot, size_t dataSize);
		D3D_DefaultConstantBuffer(const D3D_DefaultConstantBuffer&) = delete;
		~D3D_DefaultConstantBuffer() = default;

		D3D_DefaultConstantBuffer& operator=(const D3D_DefaultConstantBuffer&) = delete;

	public:
		void UploadData(size_t dataSize, const void* data);
	};

	// NOTE: Use for data that changes at least once per frame
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

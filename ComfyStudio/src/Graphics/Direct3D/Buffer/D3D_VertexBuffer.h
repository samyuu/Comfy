#pragma once
#include "../Direct3D.h"
#include "../GraphicsInterfaces.h"

namespace Comfy::Graphics
{
	class D3D_VertexBuffer : IGraphicsResource
	{
	protected:
		D3D_VertexBuffer(size_t dataSize, const void* data, size_t stride, D3D11_USAGE usage, UINT accessFlags);
		virtual ~D3D_VertexBuffer() = default;

	public:
		virtual void Bind();
		virtual void UnBind();

	public:
		ID3D11Buffer* GetBuffer();
		const D3D11_BUFFER_DESC& GetDescription();

	protected:
		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	class D3D_StaticVertexBuffer final : public D3D_VertexBuffer
	{
	public:
		D3D_StaticVertexBuffer(size_t dataSize, const void* data, size_t stride);
		~D3D_StaticVertexBuffer() = default;

	public:
	};

	class D3D_DynamicVertexBuffer final : public D3D_VertexBuffer
	{
	public:
		D3D_DynamicVertexBuffer(size_t dataSize, const void* data, size_t stride);
		~D3D_DynamicVertexBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

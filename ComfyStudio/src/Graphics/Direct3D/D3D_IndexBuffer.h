#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "Graphics/GraphicsTypes.h"

namespace Graphics
{
	class D3D_IndexBuffer : IGraphicsResource
	{
	protected:
		D3D_IndexBuffer(size_t dataSize, const void* data, IndexType indexType, D3D11_USAGE usage, UINT accessFlags);
		virtual ~D3D_IndexBuffer() = default;

	public:
		virtual void Bind();
		virtual void UnBind();

	public:
		ID3D11Buffer* GetBuffer();

	protected:
		const DXGI_FORMAT indexFormat;
		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	class D3D_StaticIndexBuffer final : public D3D_IndexBuffer
	{
	public:
		D3D_StaticIndexBuffer(size_t dataSize, const void* data, IndexType indexType);
		D3D_StaticIndexBuffer(const D3D_StaticIndexBuffer&) = delete;
		~D3D_StaticIndexBuffer() = default;

		D3D_StaticIndexBuffer& operator=(const D3D_StaticIndexBuffer&) = delete;

	public:
	};

	class D3D_DynamicIndexBuffer final : public D3D_IndexBuffer
	{
	public:
		D3D_DynamicIndexBuffer(size_t dataSize, const void* data, IndexType indexType);
		D3D_DynamicIndexBuffer(const D3D_DynamicIndexBuffer&) = delete;
		~D3D_DynamicIndexBuffer() = default;

		D3D_DynamicIndexBuffer& operator=(const D3D_DynamicIndexBuffer&) = delete;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

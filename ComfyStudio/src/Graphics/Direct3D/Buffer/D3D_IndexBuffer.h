#pragma once
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Graphics
{
	class D3D_IndexBuffer : ID3DGraphicsResource
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
		~D3D_StaticIndexBuffer() = default;

	public:
	};

	class D3D_DynamicIndexBuffer final : public D3D_IndexBuffer
	{
	public:
		D3D_DynamicIndexBuffer(size_t dataSize, const void* data, IndexType indexType);
		~D3D_DynamicIndexBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

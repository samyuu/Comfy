#pragma once
#include "../Direct3D.h"

namespace Comfy::Graphics::D3D11
{
	class VertexBuffer : IGraphicsResource
	{
	protected:
		VertexBuffer(size_t dataSize, const void* data, size_t stride, D3D11_USAGE usage, UINT accessFlags);
		virtual ~VertexBuffer() = default;

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

	class StaticVertexBuffer final : public VertexBuffer
	{
	public:
		StaticVertexBuffer(size_t dataSize, const void* data, size_t stride);
		~StaticVertexBuffer() = default;

	public:
	};

	class DynamicVertexBuffer final : public VertexBuffer
	{
	public:
		DynamicVertexBuffer(size_t dataSize, const void* data, size_t stride);
		~DynamicVertexBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

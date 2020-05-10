#pragma once
#include "../Direct3D.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Graphics::D3D11
{
	class IndexBuffer : IGraphicsResource
	{
	protected:
		IndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, D3D11_USAGE usage, UINT accessFlags);
		virtual ~IndexBuffer() = default;

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

	class StaticIndexBuffer final : public IndexBuffer
	{
	public:
		StaticIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat);
		~StaticIndexBuffer() = default;

	public:
	};

	class DynamicIndexBuffer final : public IndexBuffer
	{
	public:
		DynamicIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat);
		~DynamicIndexBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

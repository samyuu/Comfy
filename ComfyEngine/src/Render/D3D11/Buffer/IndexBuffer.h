#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "../Direct3D.h"

namespace Comfy::Render::D3D11
{
	constexpr DXGI_FORMAT IndexFormatToDXGIFormat(Graphics::IndexFormat indexFormat)
	{
		switch (indexFormat)
		{
		case Graphics::IndexFormat::U8:
			return DXGI_FORMAT_R8_UINT;
		case Graphics::IndexFormat::U16:
			return DXGI_FORMAT_R16_UINT;
		case Graphics::IndexFormat::U32:
			return DXGI_FORMAT_R32_UINT;
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	constexpr size_t GetIndexFormatSize(Graphics::IndexFormat indexFormat)
	{
		switch (indexFormat)
		{
		case Graphics::IndexFormat::U8:
			return sizeof(u8);
		case Graphics::IndexFormat::U16:
			return sizeof(u16);
		case Graphics::IndexFormat::U32:
			return sizeof(u32);
		}

		assert(false);
		return DXGI_FORMAT_UNKNOWN;
	}

	class IndexBuffer : public IGraphicsResource
	{
	protected:
		IndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat, D3D11_USAGE usage, UINT accessFlags);
		virtual ~IndexBuffer() = default;

	public:
		virtual void Bind();
		virtual void UnBind();

	public:
		ID3D11Buffer* GetBuffer();

	protected:
		DXGI_FORMAT indexFormat;
		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	class StaticIndexBuffer final : public IndexBuffer
	{
	public:
		StaticIndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat);
		~StaticIndexBuffer() = default;

	public:
	};

	class DynamicIndexBuffer final : public IndexBuffer
	{
	public:
		DynamicIndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat);
		~DynamicIndexBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};
}

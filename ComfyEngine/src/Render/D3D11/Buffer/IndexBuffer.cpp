#include "IndexBuffer.h"

namespace Comfy::Render::D3D11
{
	namespace
	{
		constexpr DXGI_FORMAT GetDXGIFormatFromIndexType(IndexFormat indexFormat)
		{
			switch (indexFormat)
			{
			case IndexFormat::U8:	return DXGI_FORMAT_R8_UINT;
			case IndexFormat::U16:	return DXGI_FORMAT_R16_UINT;
			case IndexFormat::U32:	return DXGI_FORMAT_R32_UINT;
			}

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}

		constexpr size_t GetIndexTypeSize(IndexFormat indexFormat)
		{
			switch (indexFormat)
			{
			case IndexFormat::U8:	return sizeof(u8);
			case IndexFormat::U16:	return sizeof(u16);
			case IndexFormat::U32:	return sizeof(u32);
			}

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	IndexBuffer::IndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, D3D11_USAGE usage, UINT accessFlags)
		: indexFormat(GetDXGIFormatFromIndexType(indexFormat))
	{
		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDescription.CPUAccessFlags = accessFlags;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = static_cast<UINT>(GetIndexTypeSize(indexFormat));

		D3D11_SUBRESOURCE_DATA initialResourceData = { data, 0, 0 };
		D3D.Device->CreateBuffer(&bufferDescription, (data == nullptr) ? nullptr : &initialResourceData, &buffer);
	}

	void IndexBuffer::Bind()
	{
		constexpr UINT offset = 0;
		D3D.Context->IASetIndexBuffer(buffer.Get(), indexFormat, offset);
	}

	void IndexBuffer::UnBind()
	{
		constexpr UINT offset = 0;
		D3D.Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, offset);
	}

	ID3D11Buffer* IndexBuffer::GetBuffer()
	{
		return buffer.Get();
	}

	StaticIndexBuffer::StaticIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat)
		: IndexBuffer(dataSize, data, indexFormat, D3D11_USAGE_IMMUTABLE, 0)
	{
	}

	DynamicIndexBuffer::DynamicIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat)
		: IndexBuffer(dataSize, data, indexFormat, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
	}

	void DynamicIndexBuffer::UploadData(size_t dataSize, const void* data)
	{
		assert(dataSize == bufferDescription.ByteWidth);

		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		D3D.Context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		{
			std::memcpy(mappedBuffer.pData, data, dataSize);
		}
		D3D.Context->Unmap(buffer.Get(), 0);
	}
}

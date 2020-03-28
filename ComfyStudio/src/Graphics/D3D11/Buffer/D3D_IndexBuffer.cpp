#include "D3D_IndexBuffer.h"

namespace Comfy::Graphics
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
			case IndexFormat::U8:	return sizeof(uint8_t);
			case IndexFormat::U16:	return sizeof(uint16_t);
			case IndexFormat::U32:	return sizeof(uint32_t);
			}

			assert(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D_IndexBuffer::D3D_IndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat, D3D11_USAGE usage, UINT accessFlags)
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

	void D3D_IndexBuffer::Bind()
	{
		constexpr UINT offset = 0;
		D3D.Context->IASetIndexBuffer(buffer.Get(), indexFormat, offset);
	}

	void D3D_IndexBuffer::UnBind()
	{
		constexpr UINT offset = 0;
		D3D.Context->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, offset);
	}

	ID3D11Buffer* D3D_IndexBuffer::GetBuffer()
	{
		return buffer.Get();
	}

	D3D_StaticIndexBuffer::D3D_StaticIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat)
		: D3D_IndexBuffer(dataSize, data, indexFormat, D3D11_USAGE_IMMUTABLE, 0)
	{
	}

	D3D_DynamicIndexBuffer::D3D_DynamicIndexBuffer(size_t dataSize, const void* data, IndexFormat indexFormat)
		: D3D_IndexBuffer(dataSize, data, indexFormat, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
	}

	void D3D_DynamicIndexBuffer::UploadData(size_t dataSize, const void* data)
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

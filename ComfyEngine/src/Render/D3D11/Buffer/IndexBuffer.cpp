#include "IndexBuffer.h"

namespace Comfy::Render::D3D11
{
	IndexBuffer::IndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat, D3D11_USAGE usage, UINT accessFlags)
		: indexFormat(IndexFormatToDXGIFormat(indexFormat))
	{
		std::unique_ptr<u16[]> convertedIndices = nullptr;

		// NOTE: It seems D3D11 doesn't officially support 8-bit indices despite the NVIDIA drivers supporting them
		if (indexFormat == Graphics::IndexFormat::U8 && data != nullptr)
		{
			indexFormat = Graphics::IndexFormat::U16;
			this->indexFormat = IndexFormatToDXGIFormat(indexFormat);

			const auto indexCount = (dataSize / sizeof(u8));
			const u8* inputIndices = static_cast<const u8*>(data);

			convertedIndices = std::make_unique<u16[]>(indexCount);
			for (size_t i = 0; i < indexCount; i++)
				convertedIndices[i] = static_cast<u16>(inputIndices[i]);

			dataSize = (indexCount * sizeof(u16));
			data = convertedIndices.get();
		}

		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDescription.CPUAccessFlags = accessFlags;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = static_cast<UINT>(GetIndexFormatSize(indexFormat));

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

	StaticIndexBuffer::StaticIndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat)
		: IndexBuffer(dataSize, data, indexFormat, D3D11_USAGE_IMMUTABLE, 0)
	{
	}

	DynamicIndexBuffer::DynamicIndexBuffer(size_t dataSize, const void* data, Graphics::IndexFormat indexFormat)
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

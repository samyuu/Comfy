#include "VertexBuffer.h"

namespace Comfy::Render::D3D11
{
	VertexBuffer::VertexBuffer(size_t dataSize, const void* data, size_t stride, D3D11_USAGE usage, UINT accessFlags)
	{
		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = accessFlags;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = static_cast<UINT>(stride);

		D3D11_SUBRESOURCE_DATA initialResourceData = { data, 0, 0 };
		D3D.Device->CreateBuffer(&bufferDescription, (data == nullptr) ? nullptr : &initialResourceData, &buffer);
	}

	void VertexBuffer::Bind()
	{
		// TODO: Eventually this should be controlable as well to bind multiple buffers to different attributes

		constexpr UINT startSlot = 0;
		constexpr UINT bufferCount = 1;

		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };
		std::array<UINT, bufferCount> strides = { bufferDescription.StructureByteStride };
		std::array<UINT, bufferCount> offsets = { 0 };

		D3D.Context->IASetVertexBuffers(startSlot, bufferCount, buffers.data(), strides.data(), offsets.data());
	}

	void VertexBuffer::UnBind()
	{
		constexpr UINT startSlot = 0;
		constexpr UINT bufferCount = 1;

		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };
		std::array<UINT, bufferCount> strides = { 0 };
		std::array<UINT, bufferCount> offsets = { 0 };

		D3D.Context->IASetVertexBuffers(startSlot, bufferCount, buffers.data(), strides.data(), offsets.data());
	}

	ID3D11Buffer* VertexBuffer::GetBuffer()
	{
		return buffer.Get();
	}

	const D3D11_BUFFER_DESC& VertexBuffer::GetDescription()
	{
		return bufferDescription;
	}

	StaticVertexBuffer::StaticVertexBuffer(size_t dataSize, const void* data, size_t stride)
		: VertexBuffer(dataSize, data, stride, D3D11_USAGE_IMMUTABLE, 0)
	{
	}

	DynamicVertexBuffer::DynamicVertexBuffer(size_t dataSize, const void* data, size_t stride)
		: VertexBuffer(dataSize, data, stride, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
	}

	void DynamicVertexBuffer::UploadData(size_t dataSize, const void* data)
	{
		assert(dataSize <= bufferDescription.ByteWidth);

		D3D11_MAPPED_SUBRESOURCE mappedBuffer;
		D3D.Context->Map(buffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);
		{
			std::memcpy(mappedBuffer.pData, data, dataSize);
		}
		D3D.Context->Unmap(buffer.Get(), 0);
	}
}

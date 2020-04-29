#include "ConstantBuffer.h"

namespace Comfy::Graphics::D3D11
{
	ConstantBuffer::ConstantBuffer(u32 slot, size_t dataSize, D3D11_USAGE usage, UINT accessFlags)
		: slot(slot)
	{
		assert(slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = accessFlags;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;

		D3D.Device->CreateBuffer(&bufferDescription, nullptr, &buffer);
	}

	void ConstantBuffer::BindVertexShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };

		D3D.Context->VSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void ConstantBuffer::UnBindVertexShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };

		D3D.Context->VSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void ConstantBuffer::BindPixelShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };

		D3D.Context->PSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void ConstantBuffer::UnBindPixelShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };

		D3D.Context->PSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	ID3D11Buffer* ConstantBuffer::GetBuffer()
	{
		return buffer.Get();
	}

	DefaultConstantBuffer::DefaultConstantBuffer(u32 slot, size_t dataSize)
		: ConstantBuffer(slot, dataSize, D3D11_USAGE_DEFAULT, 0)
	{
	}
	
	void DefaultConstantBuffer::UploadData(size_t dataSize, const void* data)
	{
		assert(dataSize == bufferDescription.ByteWidth);

		D3D.Context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
	}

	DynamicConstantBuffer::DynamicConstantBuffer(u32 slot, size_t dataSize)
		: ConstantBuffer(slot, dataSize, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE)
	{
	}

	void DynamicConstantBuffer::UploadData(size_t dataSize, const void* data)
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

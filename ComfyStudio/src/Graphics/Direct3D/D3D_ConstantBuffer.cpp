#include "D3D_ConstantBuffer.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	D3D_ConstantBuffer::D3D_ConstantBuffer(uint32_t slot, size_t dataSize, D3D11_USAGE usage)
		: slot(slot)
	{
		assert(slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);

		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = 0;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;

		D3D.Device->CreateBuffer(&bufferDescription, nullptr, &buffer);
	}

	void D3D_ConstantBuffer::BindVertexShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };

		D3D.Context->VSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void D3D_ConstantBuffer::UnBindVertexShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };

		D3D.Context->VSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void D3D_ConstantBuffer::BindPixelShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };

		D3D.Context->PSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	void D3D_ConstantBuffer::UnBindPixelShader()
	{
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };

		D3D.Context->PSSetConstantBuffers(slot, bufferCount, buffers.data());
	}

	D3D_DynamicConstantBuffer::D3D_DynamicConstantBuffer(uint32_t slot, size_t dataSize)
		: D3D_ConstantBuffer(slot, dataSize, D3D11_USAGE_DEFAULT)
	{
	}

	void D3D_DynamicConstantBuffer::UploadData(size_t dataSize, const void* data)
	{
		assert(dataSize == bufferDescription.ByteWidth);

		D3D.Context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
	}
}

#include "D3D_ConstantBuffer.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	D3D_ConstantBuffer::D3D_ConstantBuffer(size_t dataSize, D3D11_USAGE usage)
	{
		bufferDescription.ByteWidth = static_cast<UINT>(dataSize);
		bufferDescription.Usage = usage;
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = 0;
		bufferDescription.MiscFlags = 0;
		bufferDescription.StructureByteStride = 0;

		D3D.Device->CreateBuffer(&bufferDescription, nullptr, &buffer);
	}

	void D3D_ConstantBuffer::Bind()
	{
		constexpr UINT startSlot = 0;
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { buffer.Get() };

		D3D.Context->VSSetConstantBuffers(startSlot, bufferCount, buffers.data());
	}

	void D3D_ConstantBuffer::UnBind()
	{
		constexpr UINT startSlot = 0;
		constexpr UINT bufferCount = 1;
		std::array<ID3D11Buffer*, bufferCount> buffers = { nullptr };

		D3D.Context->VSSetConstantBuffers(startSlot, bufferCount, buffers.data());
	}

	D3D_DynamicConstantBuffer::D3D_DynamicConstantBuffer(size_t dataSize)
		: D3D_ConstantBuffer(dataSize, D3D11_USAGE_DEFAULT)
	{
	}

	void D3D_DynamicConstantBuffer::UploadData(size_t dataSize, const void* data)
	{
		assert(dataSize == bufferDescription.ByteWidth);

		D3D.Context->UpdateSubresource(buffer.Get(), 0, nullptr, data, 0, 0);
	}
}

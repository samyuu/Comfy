#include "D3D11Buffer.h"
#include "D3D11GraphicsTypeHelpers.h"

namespace Comfy::Render
{
	D3D11VertexBuffer::D3D11VertexBuffer(D3D11& d3d11, size_t dataSize, const void* data, size_t stride, D3D11_USAGE usage) : D3D11RefForDeferedDeletion(d3d11)
	{
		if (usage == D3D11_USAGE_IMMUTABLE)
			assert(data != nullptr && dataSize > 0);

		BufferDesc.ByteWidth = static_cast<UINT>(dataSize);
		BufferDesc.Usage = usage;
		BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		BufferDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = static_cast<UINT>(stride);

		d3d11.Device->CreateBuffer(&BufferDesc, (data == nullptr) ? nullptr : PtrArg<D3D11_SUBRESOURCE_DATA>({ data, 0, 0 }), &Buffer);
	}

	D3D11VertexBuffer::~D3D11VertexBuffer()
	{
		D3D11RefForDeferedDeletion.DeferObjectDeletion(Buffer);
	}

	void D3D11VertexBuffer::Bind(D3D11& d3d11) const
	{
		constexpr UINT startSlot = 0;
		d3d11.ImmediateContext->IASetVertexBuffers(startSlot, 1, PtrArg(Buffer.Get()), PtrArg<UINT>(BufferDesc.StructureByteStride), PtrArg<UINT>(0));
	}

	void D3D11VertexBuffer::UnBind(D3D11& d3d11) const
	{
		constexpr UINT startSlot = 0;
		d3d11.ImmediateContext->IASetVertexBuffers(startSlot, 1, PtrArg<ID3D11Buffer*>(nullptr), PtrArg<UINT>(0), PtrArg<UINT>(0));
	}

	void D3D11VertexBuffer::UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* data)
	{
		assert(dataSize <= BufferDesc.ByteWidth);
		assert(BufferDesc.Usage == D3D11_USAGE_DYNAMIC);

		D3D11Helper::MemoryMapAndCopy(d3d11.ImmediateContext.Get(), Buffer.Get(), data, dataSize);
	}

	D3D11IndexBuffer::D3D11IndexBuffer(D3D11& d3d11, size_t dataSize, const void* data, Graphics::IndexFormat format, D3D11_USAGE usage) : D3D11RefForDeferedDeletion(d3d11)
	{
		if (usage == D3D11_USAGE_IMMUTABLE)
			assert(data != nullptr && dataSize > 0);

		std::unique_ptr<u16[]> convertedIndices = nullptr;

		// NOTE: It seems D3D11 doesn't officially support 8-bit indices despite the NVIDIA drivers supporting them
		if (format == Graphics::IndexFormat::U8 && data != nullptr)
		{
			format = Graphics::IndexFormat::U16;

			const auto indexCount = (dataSize / sizeof(u8));
			const u8* inputIndices = static_cast<const u8*>(data);

			convertedIndices = std::make_unique<u16[]>(indexCount);
			for (size_t i = 0; i < indexCount; i++)
				convertedIndices[i] = static_cast<u16>(inputIndices[i]);

			dataSize = (indexCount * sizeof(u16));
			data = convertedIndices.get();
		}
			
		DXGIFormat = IndexFormatToDXGIFormat(format);

		BufferDesc.ByteWidth = static_cast<UINT>(dataSize);
		BufferDesc.Usage = usage;
		BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		BufferDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = static_cast<UINT>(GetIndexFormatSize(format));

		d3d11.Device->CreateBuffer(&BufferDesc, (data == nullptr) ? nullptr : PtrArg<D3D11_SUBRESOURCE_DATA>({ data, 0, 0 }), &Buffer);
	}

	D3D11IndexBuffer::~D3D11IndexBuffer()
	{
		D3D11RefForDeferedDeletion.DeferObjectDeletion(Buffer);
	}

	void D3D11IndexBuffer::Bind(D3D11& d3d11) const
	{
		constexpr UINT offset = 0;
		d3d11.ImmediateContext->IASetIndexBuffer(Buffer.Get(), DXGIFormat, offset);
	}

	void D3D11IndexBuffer::UnBind(D3D11& d3d11) const
	{
		constexpr UINT offset = 0;
		d3d11.ImmediateContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, offset);
	}

	void D3D11IndexBuffer::UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* data)
	{
		assert(dataSize == BufferDesc.ByteWidth);
		assert(BufferDesc.Usage == D3D11_USAGE_DYNAMIC);

		D3D11Helper::MemoryMapAndCopy(d3d11.ImmediateContext.Get(), Buffer.Get(), data, dataSize);
	}

	D3D11ConstantBuffer::D3D11ConstantBuffer(D3D11& d3d11, u32 slot, size_t dataSize, D3D11_USAGE usage)
	{
		assert(slot < D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT);
		assert(usage == D3D11_USAGE_DEFAULT || usage == D3D11_USAGE_DYNAMIC);

		Slot = slot;
		BufferDesc.ByteWidth = static_cast<UINT>(dataSize);
		BufferDesc.Usage = usage;
		BufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		BufferDesc.CPUAccessFlags = (usage == D3D11_USAGE_DYNAMIC) ? D3D11_CPU_ACCESS_WRITE : 0;
		BufferDesc.MiscFlags = 0;
		BufferDesc.StructureByteStride = 0;

		d3d11.Device->CreateBuffer(&BufferDesc, nullptr, &Buffer);
	}

	void D3D11ConstantBuffer::BindVertexShader(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->VSSetConstantBuffers(Slot, 1, PtrArg<ID3D11Buffer*>(Buffer.Get()));
	}

	void D3D11ConstantBuffer::UnBindVertexShader(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->VSSetConstantBuffers(Slot, 1, PtrArg<ID3D11Buffer*>(nullptr));
	}

	void D3D11ConstantBuffer::BindPixelShader(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->PSSetConstantBuffers(Slot, 1, PtrArg<ID3D11Buffer*>(Buffer.Get()));
	}

	void D3D11ConstantBuffer::UnBindPixelShader(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->PSSetConstantBuffers(Slot, 1, PtrArg<ID3D11Buffer*>(nullptr));
	}

	void D3D11ConstantBuffer::UploadData(D3D11& d3d11, size_t dataSize, const void* data)
	{
		assert(dataSize == BufferDesc.ByteWidth);

		if (BufferDesc.Usage == D3D11_USAGE_DEFAULT)
			d3d11.ImmediateContext->UpdateSubresource(Buffer.Get(), 0, nullptr, data, 0, 0);
		else
			D3D11Helper::MemoryMapAndCopy(d3d11.ImmediateContext.Get(), Buffer.Get(), data, dataSize);
	}
}

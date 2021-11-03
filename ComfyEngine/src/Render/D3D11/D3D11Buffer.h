#pragma once
#include "Types.h"
#include "D3D11.h"
#include "D3D11OpaqueResource.h"
#include "Graphics/GraphicTypes.h"

namespace Comfy::Render
{
	struct D3D11VertexBuffer : public D3D11OpaqueResource, NonCopyable
	{
		D3D11VertexBuffer(D3D11& d3d11, size_t dataSize, const void* data, size_t stride, D3D11_USAGE usage);
		~D3D11VertexBuffer() override;

		void Bind(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;
		void UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* data);

		D3D11& D3D11RefForDeferedDeletion;
		D3D11_BUFFER_DESC BufferDesc = {};
		ComPtr<ID3D11Buffer> Buffer = {};
	};

	struct D3D11IndexBuffer : public D3D11OpaqueResource, NonCopyable
	{
		D3D11IndexBuffer(D3D11& d3d11, size_t dataSize, const void* data, Graphics::IndexFormat format, D3D11_USAGE usage);
		~D3D11IndexBuffer() override;

		void Bind(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;
		void UploadDataIfDynamic(D3D11& d3d11, size_t dataSize, const void* data);

		D3D11& D3D11RefForDeferedDeletion;
		DXGI_FORMAT DXGIFormat = {};
		D3D11_BUFFER_DESC BufferDesc = {};
		ComPtr<ID3D11Buffer> Buffer = {};
	};

	// NOTE: Use D3D11_USAGE_DEFAULT for data that changes *less than* once per frame
	//		 and D3D11_USAGE_DYNAMIC for data that changes *at least* once per frame
	struct D3D11ConstantBuffer : NonCopyable
	{
		static constexpr size_t DataAlignmentRequirement = 16;

		D3D11ConstantBuffer(D3D11& d3d11, u32 slot, size_t dataSize, D3D11_USAGE usage);
		~D3D11ConstantBuffer() = default;

		void BindVertexShader(D3D11& d3d11) const;
		void UnBindVertexShader(D3D11& d3d11) const;

		void BindPixelShader(D3D11& d3d11) const;
		void UnBindPixelShader(D3D11& d3d11) const;

		void UploadData(D3D11& d3d11, size_t dataSize, const void* data);

		u32 Slot = {};
		D3D11_BUFFER_DESC BufferDesc = {};
		ComPtr<ID3D11Buffer> Buffer = {};
	};

	template <typename ConstantBufferStruct>
	struct D3D11ConstantBufferTemplate : NonCopyable
	{
		static_assert(sizeof(ConstantBufferStruct) % D3D11ConstantBuffer::DataAlignmentRequirement == 0);

	public:
		D3D11ConstantBufferTemplate(D3D11& d3d11, u32 slot, D3D11_USAGE usage) : Data(), Buffer(d3d11, slot, sizeof(Data), usage) {}
		D3D11ConstantBufferTemplate(D3D11& d3d11, u32 slot, D3D11_USAGE usage, const char* debugName) : D3D11ConstantBufferTemplate(d3d11, slot, usage) { D3D11_SetObjectDebugName(Buffer.Buffer.Get(), debugName); }

	public:
		inline void BindVertexShader(D3D11& d3d11) const { Buffer.BindVertexShader(d3d11); }
		inline void UnBindVertexShader(D3D11& d3d11) const { Buffer.UnBindVertexShader(d3d11); }

		inline void BindPixelShader(D3D11& d3d11) const { Buffer.BindPixelShader(d3d11); }
		inline void UnBindPixelShader(D3D11& d3d11) const { Buffer.UnBindPixelShader(d3d11); }

		inline void BindShaders(D3D11& d3d11) const { BindVertexShader(d3d11); BindPixelShader(d3d11); }
		inline void UnBindShaders(D3D11& d3d11) const { UnBindVertexShader(d3d11); UnBindPixelShader(d3d11); }

		inline void UploadData(D3D11& d3d11) { Buffer.UploadData(d3d11, sizeof(ConstantBufferStruct), &Data); }

	public:
		ConstantBufferStruct Data = {};
		D3D11ConstantBuffer Buffer = {};
	};
}

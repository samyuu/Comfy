#pragma once
#include "../Direct3D.h"

namespace Comfy::Graphics
{
	class D3D_ConstantBuffer : IGraphicsResource
	{
	public:
		static constexpr size_t DataAlignmentRequirement = 16;

	protected:
		D3D_ConstantBuffer(uint32_t slot, size_t dataSize, D3D11_USAGE usage, UINT accessFlags);
		virtual ~D3D_ConstantBuffer() = default;

	public:
		// TODO: Static methods to bind multiple buffers at once and reduce API overhead
		virtual void BindVertexShader();
		virtual void UnBindVertexShader();

		virtual void BindPixelShader();
		virtual void UnBindPixelShader();

	public:
		ID3D11Buffer* GetBuffer();

	protected:
		uint32_t slot;

		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	// NOTE: Use for data that changes less than once per frame
	class D3D_DefaultConstantBuffer final : public D3D_ConstantBuffer
	{
	public:
		D3D_DefaultConstantBuffer(uint32_t slot, size_t dataSize);
		~D3D_DefaultConstantBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};

	// NOTE: Use for data that changes at least once per frame
	class D3D_DynamicConstantBuffer final : public D3D_ConstantBuffer
	{
	public:
		D3D_DynamicConstantBuffer(uint32_t slot, size_t dataSize);
		~D3D_DynamicConstantBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data);
	};

	template <typename CBType, typename DataType>
	class D3D_ConstantBufferTemplate
	{
		static_assert(std::is_base_of<D3D_ConstantBuffer, CBType>::value);
		static_assert(sizeof(DataType) % D3D_ConstantBuffer::DataAlignmentRequirement == 0);

	public:
		D3D_ConstantBufferTemplate(uint32_t slot) : Data(), Buffer(slot, sizeof(DataType)) {};
		D3D_ConstantBufferTemplate(uint32_t slot, const char* debugName) : Data(), Buffer(slot, sizeof(DataType)) { D3D_SetObjectDebugName(Buffer.GetBuffer(), debugName); };

	public:
		inline void BindVertexShader() { Buffer.BindVertexShader(); };
		inline void UnBindVertexShader() { Buffer.UnBindVertexShader(); };

		inline void BindPixelShader() { Buffer.BindPixelShader(); };
		inline void UnBindPixelShader() { Buffer.UnBindPixelShader(); };

		inline void BindShaders() { BindVertexShader(); BindPixelShader(); };
		inline void UnBindShaders() { UnBindVertexShader(); UnBindPixelShader(); };

		inline void UploadData() { Buffer.UploadData(sizeof(DataType), &Data); };

	public:
		DataType Data;
		CBType Buffer;
	};

	template <typename DataType>
	using D3D_DefaultConstantBufferTemplate = D3D_ConstantBufferTemplate<D3D_DefaultConstantBuffer, DataType>;

	template <typename DataType>
	using D3D_DynamicConstantBufferTemplate = D3D_ConstantBufferTemplate<D3D_DynamicConstantBuffer, DataType>;
}

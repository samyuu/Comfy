#pragma once
#include "Types.h"
#include "../Direct3D.h"

namespace Comfy::Render::D3D11
{
	class ConstantBuffer : IGraphicsResource
	{
	public:
		static constexpr size_t DataAlignmentRequirement = 16;

	protected:
		ConstantBuffer(u32 slot, size_t dataSize, D3D11_USAGE usage, UINT accessFlags);
		virtual ~ConstantBuffer() = default;

	public:
		// TODO: Static methods to bind multiple buffers at once and reduce API overhead
		virtual void BindVertexShader();
		virtual void UnBindVertexShader();

		virtual void BindPixelShader();
		virtual void UnBindPixelShader();

		virtual void UploadData(size_t dataSize, const void* data) = 0;

	public:
		ID3D11Buffer* GetBuffer();

	protected:
		u32 slot;

		D3D11_BUFFER_DESC bufferDescription;
		ComPtr<ID3D11Buffer> buffer = nullptr;
	};

	// NOTE: Use for data that changes less than once per frame
	class DefaultConstantBuffer final : public ConstantBuffer
	{
	public:
		DefaultConstantBuffer(u32 slot, size_t dataSize);
		~DefaultConstantBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data) override;
	};

	// NOTE: Use for data that changes at least once per frame
	class DynamicConstantBuffer final : public ConstantBuffer
	{
	public:
		DynamicConstantBuffer(u32 slot, size_t dataSize);
		~DynamicConstantBuffer() = default;

	public:
		void UploadData(size_t dataSize, const void* data) override;
	};

	template <typename CBType, typename DataType>
	class ConstantBufferTemplate
	{
		static_assert(std::is_base_of<ConstantBuffer, CBType>::value);
		static_assert(sizeof(DataType) % ConstantBuffer::DataAlignmentRequirement == 0);

	public:
		ConstantBufferTemplate(u32 slot) : Data(), Buffer(slot, sizeof(DataType)) {}
		ConstantBufferTemplate(u32 slot, const char* debugName) : Data(), Buffer(slot, sizeof(DataType)) { D3D11_SetObjectDebugName(Buffer.GetBuffer(), debugName); }

	public:
		inline void BindVertexShader() { Buffer.BindVertexShader(); }
		inline void UnBindVertexShader() { Buffer.UnBindVertexShader(); }

		inline void BindPixelShader() { Buffer.BindPixelShader(); }
		inline void UnBindPixelShader() { Buffer.UnBindPixelShader(); }

		inline void BindShaders() { BindVertexShader(); BindPixelShader(); }
		inline void UnBindShaders() { UnBindVertexShader(); UnBindPixelShader(); }

		inline void UploadData() { Buffer.UploadData(sizeof(DataType), &Data); }

	public:
		DataType Data;
		CBType Buffer;
	};

	template <typename DataType>
	using DefaultConstantBufferTemplate = ConstantBufferTemplate<DefaultConstantBuffer, DataType>;

	template <typename DataType>
	using DynamicConstantBufferTemplate = ConstantBufferTemplate<DynamicConstantBuffer, DataType>;
}

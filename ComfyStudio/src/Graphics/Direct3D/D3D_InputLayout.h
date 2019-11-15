#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "D3D_Shader.h"

namespace Graphics
{
	struct InputElement
	{
		const char* SemanticName;
		uint32_t SemanticIndex;
		DXGI_FORMAT Format;
		uint32_t ByteOffset;
	};

	// TODO: Input layout class that supports multiple vertex buffers
	class D3D_InputLayout final : IGraphicsResource
	{
	public:
		D3D_InputLayout(const InputElement* elements, size_t elementCount, const D3D_VertexShader& vertexShader);
		D3D_InputLayout(const D3D_InputLayout&) = delete;
		~D3D_InputLayout() = default;

		D3D_InputLayout& operator=(const D3D_InputLayout&) = delete;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11InputLayout* GetLayout();

	private:
		// NOTE: No need to dynamically allocate additional memory when there are only ever a small amount of elements
		static constexpr size_t maxElementCount = 8;

		const size_t usedElementCount;
		std::array<D3D11_INPUT_ELEMENT_DESC, maxElementCount> elementDescriptions;

		ComPtr<ID3D11InputLayout> layout;
	};
}

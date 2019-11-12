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
		D3D_InputLayout(std::initializer_list<InputElement> elements, const D3D_VertexShader& vertexShader);
		virtual ~D3D_InputLayout() = default;

	public:
		void Bind();
		void UnBind();

	protected:
		// NOTE: No need to dynamically allocate additional memory when there are only ever a small amount of elements
		static constexpr size_t maxElementCount = 8;

		const size_t usedElementCount;
		std::array<D3D11_INPUT_ELEMENT_DESC, maxElementCount> elementDescriptions;

		ComPtr<ID3D11InputLayout> layout;
	};
}

#pragma once
#include "../Direct3D.h"
#include "../Shader/Shader.h"

namespace Comfy::Graphics
{
	// TODO: Use easier more limited custom format enum instead of the DXGI format type
	struct InputElement
	{
		const char* SemanticName;
		uint32_t SemanticIndex;
		DXGI_FORMAT Format;
		uint32_t ByteOffset;
		uint32_t InputSlot = 0;
	};

	class D3D_InputLayout final : ID3DGraphicsResource
	{
	public:
		D3D_InputLayout(const InputElement* elements, size_t elementCount, const D3D_VertexShader& vertexShader);
		~D3D_InputLayout() = default;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11InputLayout* GetLayout();

	private:
		// NOTE: No need to dynamically allocate additional memory when there are only ever a small fixed amount of elements
		static constexpr size_t maxElementCount = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;

		const size_t usedElementCount;
		std::array<D3D11_INPUT_ELEMENT_DESC, maxElementCount> elementDescriptions;

		ComPtr<ID3D11InputLayout> layout;
	};
}

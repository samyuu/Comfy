#pragma once
#include "Types.h"
#include "../Direct3D.h"
#include "../Shader/Shader.h"

namespace Comfy::Graphics::D3D11
{
	// TODO: Use easier more limited custom format enum instead of the DXGI format type
	struct InputElement
	{
		const char* SemanticName;
		u32 SemanticIndex;
		DXGI_FORMAT Format;
		u32 ByteOffset;
		u32 InputSlot = 0;
	};

	class InputLayout final : IGraphicsResource
	{
	public:
		InputLayout(const InputElement* elements, size_t elementCount, const VertexShader& vertexShader);
		~InputLayout() = default;

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

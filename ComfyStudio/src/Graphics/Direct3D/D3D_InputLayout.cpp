#include "D3D_InputLayout.h"

namespace Graphics
{
	D3D_InputLayout::D3D_InputLayout(std::initializer_list<InputElement> elements, const D3D_VertexShader& vertexShader)
		: usedElementCount(elements.size())
	{
		assert(usedElementCount <= maxElementCount);

		size_t elementIndex = 0;
		for (const auto& element : elements)
		{
			D3D11_INPUT_ELEMENT_DESC& description = elementDescriptions[elementIndex++];
			description.SemanticName = element.SemanticName;
			description.SemanticIndex = element.SemanticIndex;
			description.Format = element.Format;
			description.InputSlot = 0;
			description.AlignedByteOffset = element.ByteOffset;
			description.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			description.InstanceDataStepRate = 0;
		}

		D3D.Device->CreateInputLayout(
			elementDescriptions.data(),
			static_cast<UINT>(usedElementCount),
			vertexShader.GetBytecode()->GetBufferPointer(), 
			vertexShader.GetBytecode()->GetBufferSize(), 
			&layout);
	}

	void D3D_InputLayout::Bind()
	{
		D3D.Context->IASetInputLayout(layout.Get());
	}
	
	void D3D_InputLayout::UnBind()
	{
		D3D.Context->IASetInputLayout(nullptr);
	}
}

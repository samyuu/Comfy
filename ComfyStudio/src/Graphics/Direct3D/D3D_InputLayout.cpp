#include "D3D_InputLayout.h"

namespace Graphics
{
	D3D_InputLayout::D3D_InputLayout(const InputElement* elements, size_t elementCount, const D3D_VertexShader& vertexShader)
		: usedElementCount(elementCount)
	{
		assert(usedElementCount <= maxElementCount);

		for (size_t i = 0; i < usedElementCount; i++)
		{
			const auto& inputElement = elements[i];
			auto& description = elementDescriptions[i];
			
			description.SemanticName = inputElement.SemanticName;
			description.SemanticIndex = inputElement.SemanticIndex;
			description.Format = inputElement.Format;
			description.InputSlot = 0;
			description.AlignedByteOffset = inputElement.ByteOffset;
			description.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
			description.InstanceDataStepRate = 0;
		}

		D3D.Device->CreateInputLayout(
			elementDescriptions.data(),
			static_cast<UINT>(usedElementCount),
			vertexShader.GetBytecodeBlob().Bytecode, 
			vertexShader.GetBytecodeBlob().Size, 
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

	ID3D11InputLayout* D3D_InputLayout::GetLayout()
	{
		return layout.Get();
	}
}

#include "InputLayout.h"

namespace Comfy::Render::D3D11
{
	InputLayout::InputLayout(const InputElement* elements, size_t elementCount, const VertexShader& vertexShader)
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
			description.InputSlot = inputElement.InputSlot;
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

	void InputLayout::Bind()
	{
		D3D.Context->IASetInputLayout(layout.Get());
	}
	
	void InputLayout::UnBind()
	{
		D3D.Context->IASetInputLayout(nullptr);
	}

	ID3D11InputLayout* InputLayout::GetLayout()
	{
		return layout.Get();
	}
}

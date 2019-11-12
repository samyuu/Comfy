#include "D3D_Shader.h"
#include "Misc/StringHelper.h"
#include <d3dcompiler.h>

namespace Graphics
{
	D3D_Shader::D3D_Shader(std::string_view bytecodeFilePath)
	{
		std::wstring widePath = Utilities::Utf8ToUtf16(bytecodeFilePath);

		HRESULT result = D3DReadFileToBlob(widePath.c_str(), &bytecodeBlob);
		assert(!FAILED(result));
	}

	ID3DBlob* D3D_Shader::GetBytecode() const
	{
		return bytecodeBlob.Get();
	}
	
	D3D_VertexShader::D3D_VertexShader(std::string_view bytecodeFilePath)
		: D3D_Shader(bytecodeFilePath)
	{
		D3D.Device->CreateVertexShader(bytecodeBlob->GetBufferPointer(), bytecodeBlob->GetBufferSize(), nullptr, &shader);
	}

	void D3D_VertexShader::Bind()
	{
		D3D.Context->VSSetShader(shader.Get(), nullptr, 0);
	}

	void D3D_VertexShader::UnBind()
	{
		D3D.Context->VSSetShader(nullptr, nullptr, 0);
	}
	
	D3D_PixelShader::D3D_PixelShader(std::string_view bytecodeFilePath)
		: D3D_Shader(bytecodeFilePath)
	{
		D3D.Device->CreatePixelShader(bytecodeBlob->GetBufferPointer(), bytecodeBlob->GetBufferSize(), nullptr, &shader);
	}

	void D3D_PixelShader::Bind()
	{
		D3D.Context->PSSetShader(shader.Get(), nullptr, 0);
	}
	
	void D3D_PixelShader::UnBind()
	{
		D3D.Context->PSSetShader(nullptr, nullptr, 0);
	}
}

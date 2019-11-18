#include "D3D_Shader.h"

namespace Graphics
{
	D3D_Shader::D3D_Shader(BytecodeBlob bytecodeBlob)
		: bytecodeBlob(bytecodeBlob)
	{
	}

	BytecodeBlob D3D_Shader::GetBytecodeBlob() const
	{
		return bytecodeBlob;
	}
	
	D3D_VertexShader::D3D_VertexShader(BytecodeBlob bytecodeBlob)
		: D3D_Shader(bytecodeBlob)
	{
		D3D.Device->CreateVertexShader(bytecodeBlob.Bytecode, bytecodeBlob.Size, nullptr, &shader);
	}

	void D3D_VertexShader::Bind()
	{
		D3D.Context->VSSetShader(shader.Get(), nullptr, 0);
	}

	void D3D_VertexShader::UnBind()
	{
		D3D.Context->VSSetShader(nullptr, nullptr, 0);
	}

	ID3D11VertexShader* D3D_VertexShader::GetShader()
	{
		return shader.Get();
	}
	
	D3D_PixelShader::D3D_PixelShader(BytecodeBlob bytecodeBlob)
		: D3D_Shader(bytecodeBlob)
	{
		D3D.Device->CreatePixelShader(bytecodeBlob.Bytecode, bytecodeBlob.Size, nullptr, &shader);
	}

	void D3D_PixelShader::Bind()
	{
		D3D.Context->PSSetShader(shader.Get(), nullptr, 0);
	}
	
	void D3D_PixelShader::UnBind()
	{
		D3D.Context->PSSetShader(nullptr, nullptr, 0);
	}

	ID3D11PixelShader* D3D_PixelShader::GetShader()
	{
		return shader.Get();
	}
}

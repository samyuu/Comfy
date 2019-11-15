#include "D3D_Shader.h"

namespace Graphics
{
	D3D_Shader::D3D_Shader(const void* bytecode, size_t byteSize)
		: bytecodeBlob(bytecode), bytecodeSize(byteSize)
	{
	}

	const void* D3D_Shader::GetBytecode() const
	{
		return bytecodeBlob;
	}

	size_t D3D_Shader::GetBytecodeSize() const
	{
		return bytecodeSize;
	}
	
	D3D_VertexShader::D3D_VertexShader(const void* bytecode, size_t byteSize)
		: D3D_Shader(bytecode, byteSize)
	{
		D3D.Device->CreateVertexShader(bytecode, byteSize, nullptr, &shader);
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
	
	D3D_PixelShader::D3D_PixelShader(const void* bytecode, size_t byteSize)
		: D3D_Shader(bytecode, byteSize)
	{
		D3D.Device->CreatePixelShader(bytecode, byteSize, nullptr, &shader);
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

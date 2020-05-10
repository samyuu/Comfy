#include "Shader.h"

namespace Comfy::Render::D3D11
{
	Shader::Shader(BytecodeBlob bytecodeBlob)
		: bytecodeBlob(bytecodeBlob)
	{
	}

	BytecodeBlob Shader::GetBytecodeBlob() const
	{
		return bytecodeBlob;
	}
	
	VertexShader::VertexShader(BytecodeBlob bytecodeBlob)
		: Shader(bytecodeBlob)
	{
		D3D.Device->CreateVertexShader(bytecodeBlob.Bytecode, bytecodeBlob.Size, nullptr, &shader);
	}

	void VertexShader::Bind() const
	{
		D3D.Context->VSSetShader(shader.Get(), nullptr, 0);
	}

	void VertexShader::UnBind() const
	{
		D3D.Context->VSSetShader(nullptr, nullptr, 0);
	}

	ID3D11VertexShader* VertexShader::GetShader()
	{
		return shader.Get();
	}
	
	PixelShader::PixelShader(BytecodeBlob bytecodeBlob)
		: Shader(bytecodeBlob)
	{
		D3D.Device->CreatePixelShader(bytecodeBlob.Bytecode, bytecodeBlob.Size, nullptr, &shader);
	}

	void PixelShader::Bind() const
	{
		D3D.Context->PSSetShader(shader.Get(), nullptr, 0);
	}
	
	void PixelShader::UnBind() const
	{
		D3D.Context->PSSetShader(nullptr, nullptr, 0);
	}

	ID3D11PixelShader* PixelShader::GetShader()
	{
		return shader.Get();
	}

	ShaderPair::ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode) 
		: VS(vsBytecode), PS(psBytecode)
	{
	}

	ShaderPair::ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode, const char* debugName)
		: VS(vsBytecode), PS(psBytecode)
	{
		D3D11_SetObjectDebugName(VS.GetShader(), "%s%s", debugName, "_VS");
		D3D11_SetObjectDebugName(PS.GetShader(), "%s%s", debugName, "_PS");
	}
	
	void ShaderPair::Bind() const
	{
		VS.Bind();
		PS.Bind();
	}

	void ShaderPair::UnBind() const
	{
		VS.UnBind();
		PS.UnBind();
	}
}

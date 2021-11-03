#include "D3D11Shader.h"

namespace Comfy::Render
{
	D3D11VertexShader::D3D11VertexShader(D3D11& d3d11, D3D11BytecodeView bytecode)
		: BytecodeView(bytecode)
	{
		d3d11.Device->CreateVertexShader(bytecode.Bytes, bytecode.Size, nullptr, &Shader);
	}

	void D3D11VertexShader::Bind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->VSSetShader(Shader.Get(), nullptr, 0);
	}

	void D3D11VertexShader::UnBind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->VSSetShader(nullptr, nullptr, 0);
	}

	D3D11PixelShader::D3D11PixelShader(D3D11& d3d11, D3D11BytecodeView bytecode)
		: BytecodeView(bytecode)
	{
		d3d11.Device->CreatePixelShader(bytecode.Bytes, bytecode.Size, nullptr, &Shader);
	}

	void D3D11PixelShader::Bind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->PSSetShader(Shader.Get(), nullptr, 0);
	}

	void D3D11PixelShader::UnBind(D3D11& d3d11) const
	{
		d3d11.ImmediateContext->PSSetShader(nullptr, nullptr, 0);
	}

	D3D11ShaderPair::D3D11ShaderPair(D3D11& d3d11, D3D11BytecodeView vsBytecode, D3D11BytecodeView psBytecode)
		: VS(d3d11, vsBytecode), PS(d3d11, psBytecode)
	{
	}

	D3D11ShaderPair::D3D11ShaderPair(D3D11& d3d11, D3D11BytecodeView vsBytecode, D3D11BytecodeView psBytecode, const char* debugName)
		: VS(d3d11, vsBytecode), PS(d3d11, psBytecode)
	{
		D3D11_SetObjectDebugName(VS.Shader.Get(), "%s%s", debugName, "_VS");
		D3D11_SetObjectDebugName(PS.Shader.Get(), "%s%s", debugName, "_PS");
	}

	void D3D11ShaderPair::Bind(D3D11& d3d11) const
	{
		VS.Bind(d3d11);
		PS.Bind(d3d11);
	}

	void D3D11ShaderPair::UnBind(D3D11& d3d11) const
	{
		VS.UnBind(d3d11);
		PS.UnBind(d3d11);
	}
}

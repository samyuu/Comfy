#pragma once
#include "Types.h"
#include "D3D11.h"

namespace Comfy::Render
{
	struct D3D11BytecodeView
	{
		const void* Bytes;
		const size_t Size;
	};

	struct D3D11VertexShader : NonCopyable
	{
		D3D11VertexShader(D3D11& d3d11, D3D11BytecodeView bytecode);
		~D3D11VertexShader() = default;

		void Bind(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;

		D3D11BytecodeView BytecodeView = {};
		ComPtr<ID3D11VertexShader> Shader = {};
	};

	struct D3D11PixelShader : NonCopyable
	{
		D3D11PixelShader(D3D11& d3d11, D3D11BytecodeView bytecode);
		~D3D11PixelShader() = default;

		void Bind(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;

		D3D11BytecodeView BytecodeView = {};
		ComPtr<ID3D11PixelShader> Shader = {};
	};

	struct D3D11ShaderPair : NonCopyable
	{
		D3D11ShaderPair(D3D11& d3d11, D3D11BytecodeView vsBytecode, D3D11BytecodeView psBytecode);
		D3D11ShaderPair(D3D11& d3d11, D3D11BytecodeView vsBytecode, D3D11BytecodeView psBytecode, const char* debugName);
		~D3D11ShaderPair() = default;

		void Bind(D3D11& d3d11) const;
		void UnBind(D3D11& d3d11) const;

		D3D11VertexShader VS;
		D3D11PixelShader PS;
	};
}

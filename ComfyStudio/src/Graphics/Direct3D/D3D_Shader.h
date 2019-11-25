#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	struct BytecodeBlob
	{
		const void* Bytecode;
		const size_t Size;
	};

	class D3D_Shader : IGraphicsResource
	{
	protected:
		D3D_Shader(BytecodeBlob bytecodeBlob);
		virtual ~D3D_Shader() = default;

	public:
		BytecodeBlob GetBytecodeBlob() const;

	private:
		BytecodeBlob bytecodeBlob;
	};

	class D3D_VertexShader final : public D3D_Shader
	{
	public:
		D3D_VertexShader(BytecodeBlob bytecodeBlob);
		D3D_VertexShader(const D3D_VertexShader&) = delete;
		~D3D_VertexShader() = default;

		D3D_VertexShader& operator=(const D3D_VertexShader&) = delete;

	public:
		void Bind() const;
		void UnBind() const;

	public:
		ID3D11VertexShader* GetShader();

	private:
		ComPtr<ID3D11VertexShader> shader;
	};

	class D3D_PixelShader final : public D3D_Shader
	{
	public:
		D3D_PixelShader(BytecodeBlob bytecodeBlob);
		D3D_PixelShader(const D3D_PixelShader&) = delete;
		~D3D_PixelShader() = default;

		D3D_PixelShader& operator=(const D3D_PixelShader&) = delete;

	public:
		void Bind() const;
		void UnBind() const;

	public:
		ID3D11PixelShader* GetShader();

	private:
		ComPtr<ID3D11PixelShader> shader;
	};

	struct D3D_ShaderPair
	{
	public:
		D3D_ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode);

	public:
		void Bind() const;
		void UnBind() const;

	public:
		D3D_VertexShader VS;
		D3D_PixelShader PS;
	};
}

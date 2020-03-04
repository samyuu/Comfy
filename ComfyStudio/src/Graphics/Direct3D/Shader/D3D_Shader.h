#pragma once
#include "../Direct3D.h"

namespace Comfy::Graphics
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
		~D3D_VertexShader() = default;

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
		~D3D_PixelShader() = default;

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
		D3D_ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode, const char* debugName);

	public:
		void Bind() const;
		void UnBind() const;

	public:
		D3D_VertexShader VS;
		D3D_PixelShader PS;
	};
}

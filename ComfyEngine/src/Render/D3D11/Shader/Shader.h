#pragma once
#include "../Direct3D.h"

namespace Comfy::Render::D3D11
{
	struct BytecodeBlob
	{
		const void* Bytecode;
		const size_t Size;
	};

	class Shader : IGraphicsResource
	{
	protected:
		Shader(BytecodeBlob bytecodeBlob);
		virtual ~Shader() = default;

	public:
		BytecodeBlob GetBytecodeBlob() const;

	private:
		BytecodeBlob bytecodeBlob;
	};

	class VertexShader final : public Shader
	{
	public:
		VertexShader(BytecodeBlob bytecodeBlob);
		~VertexShader() = default;

	public:
		void Bind() const;
		void UnBind() const;

	public:
		ID3D11VertexShader* GetShader();

	private:
		ComPtr<ID3D11VertexShader> shader;
	};

	class PixelShader final : public Shader
	{
	public:
		PixelShader(BytecodeBlob bytecodeBlob);
		~PixelShader() = default;

	public:
		void Bind() const;
		void UnBind() const;

	public:
		ID3D11PixelShader* GetShader();

	private:
		ComPtr<ID3D11PixelShader> shader;
	};

	struct ShaderPair
	{
	public:
		ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode);
		ShaderPair(BytecodeBlob vsBytecode, BytecodeBlob psBytecode, const char* debugName);

	public:
		void Bind() const;
		void UnBind() const;

	public:
		VertexShader VS;
		PixelShader PS;
	};
}

#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"

namespace Graphics
{
	class D3D_Shader : IGraphicsResource
	{
	protected:
		D3D_Shader(const void* bytecodeBlob, size_t byteSize);
		virtual ~D3D_Shader() = default;

	public:
		const void* GetBytecode() const;
		size_t GetBytecodeSize() const;

	private:
		const void* bytecodeBlob;
		const size_t bytecodeSize;
	};

	class D3D_VertexShader final : public D3D_Shader
	{
	public:
		D3D_VertexShader(const void* bytecode, size_t byteSize);
		D3D_VertexShader(const D3D_VertexShader&) = delete;
		~D3D_VertexShader() = default;

		D3D_VertexShader& operator=(const D3D_VertexShader&) = delete;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11VertexShader* GetShader();

	private:
		ComPtr<ID3D11VertexShader> shader;
	};

	class D3D_PixelShader final : public D3D_Shader
	{
	public:
		D3D_PixelShader(const void* bytecode, size_t byteSize);
		D3D_PixelShader(const D3D_PixelShader&) = delete;
		~D3D_PixelShader() = default;

		D3D_PixelShader& operator=(const D3D_PixelShader&) = delete;

	public:
		void Bind();
		void UnBind();

	public:
		ID3D11PixelShader* GetShader();

	private:
		ComPtr<ID3D11PixelShader> shader;
	};
}

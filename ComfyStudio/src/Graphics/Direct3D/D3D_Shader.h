#pragma once
#include "Direct3D.h"
#include "GraphicsInterfaces.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	class D3D_Shader : IGraphicsResource
	{
	protected:
		D3D_Shader(std::string_view bytecodeFilePath);
		virtual ~D3D_Shader() = default;

	public:
		ID3DBlob* GetBytecode() const;

	protected:
		ComPtr<ID3DBlob> bytecodeBlob;
	};

	class D3D_VertexShader final : public D3D_Shader
	{
	public:
		D3D_VertexShader(std::string_view bytecodeFilePath);
		D3D_VertexShader(const D3D_VertexShader&) = delete;
		~D3D_VertexShader() = default;

		D3D_VertexShader& operator=(const D3D_VertexShader&) = delete;

	public:
		void Bind();
		void UnBind();

	protected:
		ComPtr<ID3D11VertexShader> shader;
	};

	class D3D_PixelShader final : public D3D_Shader
	{
	public:
		D3D_PixelShader(std::string_view bytecodeFilePath);
		D3D_PixelShader(const D3D_PixelShader&) = delete;
		~D3D_PixelShader() = default;

		D3D_PixelShader& operator=(const D3D_PixelShader&) = delete;

	public:
		void Bind();
		void UnBind();

	protected:
		ComPtr<ID3D11PixelShader> shader;
	};
}

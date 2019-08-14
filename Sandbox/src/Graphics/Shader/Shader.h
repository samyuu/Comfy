#pragma once
#include "ShaderProgram.h"

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- SpriteShader:
	// ------------------------------------------------------------------------------------------------

	class SpriteShader : public ShaderProgram
	{
	public:
		SpriteShader();
		SpriteShader(const SpriteShader&) = delete;
		~SpriteShader();

		Uniform ProjectionView = { UniformType::Mat4, "u_ProjectionView" };
		Uniform UseSolidColor = { UniformType::Int, "u_SolidColor" };
		Uniform UseTextShadow = { UniformType::Int, "u_TextShadow" };
		Uniform UseCheckerboard = { UniformType::Int, "u_Checkerboard" };
		Uniform CheckerboardSize = { UniformType::Vec2, "u_CheckboardSize" };
		Uniform TextureFormat = { UniformType::Int, "u_TextureFormat" };
		Uniform TextureMaskFormat = { UniformType::Int, "u_TextureMaskFormat" };
		Uniform Texture = { UniformType::Int, "u_TextureSampler" };
		Uniform TextureMask = { UniformType::Int, "u_TextureMaskSampler" };

	public:
		virtual Uniform* GetFirstUniform() override;
		virtual Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- ComfyShader:
	// ------------------------------------------------------------------------------------------------

	class ComfyShader : public ShaderProgram
	{
	public:
		ComfyShader();
		ComfyShader(const ComfyShader&) = delete;
		~ComfyShader();

		Uniform Model = { UniformType::Mat4, "u_Model" };
		Uniform View = { UniformType::Mat4, "u_View" };
		Uniform Projection = { UniformType::Mat4, "u_Projection" };
		Uniform Texture0 = { UniformType::Int, "u_TextureSampler0" };
		Uniform Texture1 = { UniformType::Int, "u_TextureSampler1" };

	public:
		virtual Uniform* GetFirstUniform() override;
		virtual Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- ScreenShader:
	// ------------------------------------------------------------------------------------------------

	class ScreenShader : public ShaderProgram
	{
	public:
		ScreenShader();
		ScreenShader(const ScreenShader&) = delete;
		~ScreenShader();

		Uniform Saturation = { UniformType::Float, "u_Saturation" };
		Uniform Brightness = { UniformType::Float, "u_Brightness" };
		Uniform ScreenTexture = { UniformType::Int, "u_ScreenTexture" };

	public:
		virtual Uniform* GetFirstUniform() override;
		virtual Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- LineShader:
	// ------------------------------------------------------------------------------------------------

	class LineShader : public ShaderProgram
	{
	public:
		LineShader();
		LineShader(const LineShader&) = delete;
		~LineShader();

		Uniform Model = { UniformType::Mat4, "u_Model" };
		Uniform View = { UniformType::Mat4, "u_View" };
		Uniform Projection = { UniformType::Mat4, "u_Projection" };

	public:
		virtual Uniform* GetFirstUniform() override;
		virtual Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
}

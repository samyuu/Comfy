#pragma once
#include "GL_ShaderProgram.h"

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- SpriteShader:
	// ------------------------------------------------------------------------------------------------

	class GL_SpriteShader : public GL_ShaderProgram
	{
	public:
		GL_SpriteShader();
		GL_SpriteShader(const GL_SpriteShader&) = delete;
		GL_SpriteShader& operator= (const GL_SpriteShader&) = delete;
		~GL_SpriteShader();

		GL_Uniform ProjectionView = { UniformType::Mat4, "u_ProjectionView" };
		GL_Uniform UseSolidColor = { UniformType::Int, "u_SolidColor" };
		GL_Uniform UseTextShadow = { UniformType::Int, "u_TextShadow" };
		GL_Uniform UseCheckerboard = { UniformType::Int, "u_Checkerboard" };
		GL_Uniform CheckerboardSize = { UniformType::Vec2, "u_CheckboardSize" };
		GL_Uniform BlendMode = { UniformType::Int, "u_BlendMode" };
		GL_Uniform TextureFormat = { UniformType::Int, "u_TextureFormat" };
		GL_Uniform TextureMaskFormat = { UniformType::Int, "u_TextureMaskFormat" };
		GL_Uniform Texture = { UniformType::Int, "u_TextureSampler" };
		GL_Uniform TextureMask = { UniformType::Int, "u_TextureMaskSampler" };

	public:
		virtual GL_Uniform* GetFirstUniform() override;
		virtual GL_Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- ComfyShader:
	// ------------------------------------------------------------------------------------------------

	class GL_ComfyShader : public GL_ShaderProgram
	{
	public:
		GL_ComfyShader();
		GL_ComfyShader(const GL_ComfyShader&) = delete;
		GL_ComfyShader& operator= (const GL_ComfyShader&) = delete;
		~GL_ComfyShader();

		GL_Uniform Model = { UniformType::Mat4, "u_Model" };
		GL_Uniform View = { UniformType::Mat4, "u_View" };
		GL_Uniform Projection = { UniformType::Mat4, "u_Projection" };
		GL_Uniform Texture0 = { UniformType::Int, "u_TextureSampler0" };
		GL_Uniform Texture1 = { UniformType::Int, "u_TextureSampler1" };

	public:
		virtual GL_Uniform* GetFirstUniform() override;
		virtual GL_Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- ScreenShader:
	// ------------------------------------------------------------------------------------------------

	class GL_ScreenShader : public GL_ShaderProgram
	{
	public:
		GL_ScreenShader();
		GL_ScreenShader(const GL_ScreenShader&) = delete;
		GL_ScreenShader& operator= (const GL_ScreenShader&) = delete;
		~GL_ScreenShader();

		GL_Uniform Saturation = { UniformType::Float, "u_Saturation" };
		GL_Uniform Brightness = { UniformType::Float, "u_Brightness" };
		GL_Uniform ScreenTexture = { UniformType::Int, "u_ScreenTexture" };

	public:
		virtual GL_Uniform* GetFirstUniform() override;
		virtual GL_Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- LineShader:
	// ------------------------------------------------------------------------------------------------

	class GL_LineShader : public GL_ShaderProgram
	{
	public:
		GL_LineShader();
		GL_LineShader(const GL_LineShader&) = delete;
		GL_LineShader& operator= (const GL_LineShader&) = delete;
		~GL_LineShader();

		GL_Uniform Model = { UniformType::Mat4, "u_Model" };
		GL_Uniform View = { UniformType::Mat4, "u_View" };
		GL_Uniform Projection = { UniformType::Mat4, "u_Projection" };

	public:
		virtual GL_Uniform* GetFirstUniform() override;
		virtual GL_Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- SimpleShader:
	// ------------------------------------------------------------------------------------------------

	class GL_SimpleShader : public GL_ShaderProgram
	{
	public:
		GL_SimpleShader();
		GL_SimpleShader(const GL_SimpleShader&) = delete;
		GL_SimpleShader& operator= (const GL_SimpleShader&) = delete;
		~GL_SimpleShader();

		GL_Uniform Model = { UniformType::Mat4, "u_Model" };
		GL_Uniform View = { UniformType::Mat4, "u_View" };
		GL_Uniform Projection = { UniformType::Mat4, "u_Projection" };

	public:
		virtual GL_Uniform* GetFirstUniform() override;
		virtual GL_Uniform* GetLastUniform() override;

		virtual const char* GetShaderName() override;
		virtual const char* GetVertexShaderPath() override;
		virtual const char* GetFragmentShaderPath() override;
	};

	// ------------------------------------------------------------------------------------------------
}

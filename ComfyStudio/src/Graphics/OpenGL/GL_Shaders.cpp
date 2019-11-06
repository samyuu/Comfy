#include "GL_Shaders.h"

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- SpriteShader:
	// ------------------------------------------------------------------------------------------------

	GL_SpriteShader::GL_SpriteShader()
	{
	}

	GL_SpriteShader::~GL_SpriteShader()
	{
	}

	GL_Uniform* GL_SpriteShader::GetFirstUniform()
	{
		return &ProjectionView;
	}

	GL_Uniform* GL_SpriteShader::GetLastUniform()
	{
		return &TextureMask;
	}

	const char* GL_SpriteShader::GetShaderName()
	{
		return "SpriteShader";
	}

	const char* GL_SpriteShader::GetVertexShaderPath()
	{
		return "shader/sprite_vert.glsl";
	}

	const char* GL_SpriteShader::GetFragmentShaderPath()
	{
		return "shader/sprite_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- ComfyShader:
	// ------------------------------------------------------------------------------------------------

	GL_ComfyShader::GL_ComfyShader()
	{
	}

	GL_ComfyShader::~GL_ComfyShader()
	{
	}

	GL_Uniform* GL_ComfyShader::GetFirstUniform()
	{
		return &Model;
	}

	GL_Uniform* GL_ComfyShader::GetLastUniform()
	{
		return &Texture1;
	}

	const char* GL_ComfyShader::GetShaderName()
	{
		return "ComfyShader";
	}

	const char* GL_ComfyShader::GetVertexShaderPath()
	{
		return "shader/comfy_vert.glsl";
	}

	const char* GL_ComfyShader::GetFragmentShaderPath()
	{
		return "shader/comfy_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- ScreenShader:
	// ------------------------------------------------------------------------------------------------

	GL_ScreenShader::GL_ScreenShader()
	{
	}

	GL_ScreenShader::~GL_ScreenShader()
	{
	}

	GL_Uniform* GL_ScreenShader::GetFirstUniform()
	{
		return &Saturation;
	}

	GL_Uniform* GL_ScreenShader::GetLastUniform()
	{
		return &ScreenTexture;
	}

	const char* GL_ScreenShader::GetShaderName()
	{
		return "ScreenShader";
	}

	const char* GL_ScreenShader::GetVertexShaderPath()
	{
		return "shader/screen_vert.glsl";
	}

	const char* GL_ScreenShader::GetFragmentShaderPath()
	{
		return "shader/screen_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- LineShader:
	// ------------------------------------------------------------------------------------------------

	GL_LineShader::GL_LineShader()
	{
	}

	GL_LineShader::~GL_LineShader()
	{
	}

	GL_Uniform* GL_LineShader::GetFirstUniform()
	{
		return &Model;
	}

	GL_Uniform* GL_LineShader::GetLastUniform()
	{
		return &Projection;
	}

	const char* GL_LineShader::GetShaderName()
	{
		return "LineShader";
	}

	const char* GL_LineShader::GetVertexShaderPath()
	{
		return "shader/line_vert.glsl";
	}

	const char* GL_LineShader::GetFragmentShaderPath()
	{
		return "shader/line_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- SimpleShader:
	// ------------------------------------------------------------------------------------------------

	GL_SimpleShader::GL_SimpleShader()
	{
	}

	GL_SimpleShader::~GL_SimpleShader()
	{
	}

	GL_Uniform* GL_SimpleShader::GetFirstUniform()
	{
		return &Model;
	}

	GL_Uniform* GL_SimpleShader::GetLastUniform()
	{
		return &Projection;
	}

	const char* GL_SimpleShader::GetShaderName()
	{
		return "SimpleShader";
	}

	const char* GL_SimpleShader::GetVertexShaderPath()
	{
		return "shader/simple_vert.glsl";
	}

	const char* GL_SimpleShader::GetFragmentShaderPath()
	{
		return "shader/simple_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
}

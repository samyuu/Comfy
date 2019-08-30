#include "Shader.h"

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- SpriteShader:
	// ------------------------------------------------------------------------------------------------

	SpriteShader::SpriteShader()
	{
	}

	SpriteShader::~SpriteShader()
	{
	}

	Uniform* SpriteShader::GetFirstUniform()
	{
		return &ProjectionView;
	}

	Uniform* SpriteShader::GetLastUniform()
	{
		return &TextureMask;
	}

	const char* SpriteShader::GetShaderName()
	{
		return "SpriteShader";
	}

	const char* SpriteShader::GetVertexShaderPath()
	{
		return "rom/shader/sprite_vert.glsl";
	}

	const char* SpriteShader::GetFragmentShaderPath()
	{
		return "rom/shader/sprite_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- ComfyShader:
	// ------------------------------------------------------------------------------------------------

	ComfyShader::ComfyShader()
	{
	}

	ComfyShader::~ComfyShader()
	{
	}

	Uniform* ComfyShader::GetFirstUniform()
	{
		return &Model;
	}

	Uniform* ComfyShader::GetLastUniform()
	{
		return &Texture1;
	}

	const char* ComfyShader::GetShaderName()
	{
		return "ComfyShader";
	}

	const char* ComfyShader::GetVertexShaderPath()
	{
		return "rom/shader/comfy_vert.glsl";
	}

	const char* ComfyShader::GetFragmentShaderPath()
	{
		return "rom/shader/comfy_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- ScreenShader:
	// ------------------------------------------------------------------------------------------------

	ScreenShader::ScreenShader()
	{
	}

	ScreenShader::~ScreenShader()
	{
	}

	Uniform* ScreenShader::GetFirstUniform()
	{
		return &Saturation;
	}

	Uniform* ScreenShader::GetLastUniform()
	{
		return &ScreenTexture;
	}

	const char* ScreenShader::GetShaderName()
	{
		return "ScreenShader";
	}

	const char* ScreenShader::GetVertexShaderPath()
	{
		return "rom/shader/screen_vert.glsl";
	}

	const char* ScreenShader::GetFragmentShaderPath()
	{
		return "rom/shader/screen_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- LineShader:
	// ------------------------------------------------------------------------------------------------

	LineShader::LineShader()
	{
	}

	LineShader::~LineShader()
	{
	}

	Uniform* LineShader::GetFirstUniform()
	{
		return &Model;
	}

	Uniform* LineShader::GetLastUniform()
	{
		return &Projection;
	}

	const char* LineShader::GetShaderName()
	{
		return "LineShader";
	}

	const char* LineShader::GetVertexShaderPath()
	{
		return "rom/shader/line_vert.glsl";
	}

	const char* LineShader::GetFragmentShaderPath()
	{
		return "rom/shader/line_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
	// --- SimpleShader:
	// ------------------------------------------------------------------------------------------------

	SimpleShader::SimpleShader()
	{
	}

	SimpleShader::~SimpleShader()
	{
	}

	Uniform* SimpleShader::GetFirstUniform()
	{
		return &Model;
	}

	Uniform* SimpleShader::GetLastUniform()
	{
		return &Projection;
	}

	const char* SimpleShader::GetShaderName()
	{
		return "SimpleShader";
	}

	const char* SimpleShader::GetVertexShaderPath()
	{
		return "rom/shader/simple_vert.glsl";
	}

	const char* SimpleShader::GetFragmentShaderPath()
	{
		return "rom/shader/simple_frag.glsl";
	}

	// ------------------------------------------------------------------------------------------------
}

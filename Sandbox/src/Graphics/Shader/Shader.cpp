#include "Shader.h"

// ------------------------------------------------------------------------------------------------
// --- SpriteShader:
// ------------------------------------------------------------------------------------------------

SpriteShader::SpriteShader()
{
}

SpriteShader::~SpriteShader()
{
}

void SpriteShader::GetAllUniformLocations()
{
	ProjectionViewLocation = GetUniformLocation("u_ProjectionView");

	UseSolidColorLocation = GetUniformLocation("u_SolidColor");
	UseTextShadowLocation = GetUniformLocation("u_TextShadow");
	UseCheckerboardLocation = GetUniformLocation("u_Checkerboard");
	CheckerboardSizeLocation = GetUniformLocation("u_CheckboardSize");
	TextureFormatLocation = GetUniformLocation("u_TextureFormat");

	TextureLocation = GetUniformLocation("u_TextureSampler");
	TextureMaskLocation = GetUniformLocation("u_TextureMaskSampler");
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

void ComfyShader::GetAllUniformLocations()
{
	ModelLocation = GetUniformLocation("u_Model");
	ViewLocation = GetUniformLocation("u_View");
	ProjectionLocation = GetUniformLocation("u_Projection");

	Texture0Location = GetUniformLocation("u_TextureSampler0");
	Texture1Location = GetUniformLocation("u_TextureSampler1");
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

void ScreenShader::GetAllUniformLocations()
{
	ScreenTextureLocation = GetUniformLocation("u_ScreenTexture");
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

void LineShader::GetAllUniformLocations()
{
	ModelLocation = GetUniformLocation("u_Model");
	ViewLocation = GetUniformLocation("u_View");
	ProjectionLocation = GetUniformLocation("u_Projection");
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

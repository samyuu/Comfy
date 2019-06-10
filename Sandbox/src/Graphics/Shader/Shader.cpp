#include "Shader.h"

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
	ModelLocation = GetUniformLocation("u_model");
	ViewLocation = GetUniformLocation("u_view");
	ProjectionLocation = GetUniformLocation("u_projection");

	Texture0Location = GetUniformLocation("textureSampler0");
	Texture1Location = GetUniformLocation("textureSampler1");
}

const char* ComfyShader::GetVertexShaderPath()
{
	return "rom/shader/test_vert.glsl";
}

const char* ComfyShader::GetFragmentShaderPath()
{
	return "rom/shader/test_frag.glsl";
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
	ScreenTextureLocation = GetUniformLocation("screenTexture");
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
	ModelLocation = GetUniformLocation("u_model");
	ViewLocation = GetUniformLocation("u_view");
	ProjectionLocation = GetUniformLocation("u_projection");
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

#include "ScreenShader.h"

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

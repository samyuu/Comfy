#include "ComfyShader.h"

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

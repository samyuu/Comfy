#include "LineShader.h"

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

#pragma once
#include "ShaderProgram.h"

class LineShader : public ShaderProgram
{
public:
	LineShader();
	~LineShader();

	UniformLocation_t ModelLocation;
	UniformLocation_t ViewLocation;
	UniformLocation_t ProjectionLocation;

protected:
	virtual void GetAllUniformLocations() override;
	virtual const char* GetVertexShaderPath() override;
	virtual const char* GetFragmentShaderPath() override;
};

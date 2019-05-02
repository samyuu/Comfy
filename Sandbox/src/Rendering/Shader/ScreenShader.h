#pragma once
#include "ShaderProgram.h"

class ScreenShader : public ShaderProgram
{
public:
	ScreenShader();
	~ScreenShader();

	UniformLocation_t ScreenTextureLocation;

protected:
	virtual void GetAllUniformLocations() override;
	virtual const char* GetVertexShaderPath() override;
	virtual const char* GetFragmentShaderPath() override;
};

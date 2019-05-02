#pragma once
#include "ShaderProgram.h"

class ComfyShader : public ShaderProgram
{
public:
	ComfyShader();
	~ComfyShader();

	UniformLocation_t ModelLocation;
	UniformLocation_t ViewLocation;
	UniformLocation_t ProjectionLocation;

	UniformLocation_t Texture0Location;
	UniformLocation_t Texture1Location;

protected:
	virtual void GetAllUniformLocations() override;
	virtual const char* GetVertexShaderPath() override;
	virtual const char* GetFragmentShaderPath() override;
};


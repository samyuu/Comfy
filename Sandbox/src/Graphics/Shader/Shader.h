#pragma once
#include "ShaderProgram.h"

// ------------------------------------------------------------------------------------------------
// --- SpriteShader:
// ------------------------------------------------------------------------------------------------

class SpriteShader : public ShaderProgram
{
public:
	SpriteShader();
	~SpriteShader();

	UniformLocation_t ProjectionLocation;

	UniformLocation_t TextureFormatLocation;
	UniformLocation_t TextureLocation;
	UniformLocation_t TextureMaskLocation;

protected:
	virtual void GetAllUniformLocations() override;
	virtual const char* GetVertexShaderPath() override;
	virtual const char* GetFragmentShaderPath() override;
};

// ------------------------------------------------------------------------------------------------
// --- ComfyShader:
// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------
// --- ScreenShader:
// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------
// --- LineShader:
// ------------------------------------------------------------------------------------------------

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

// ------------------------------------------------------------------------------------------------

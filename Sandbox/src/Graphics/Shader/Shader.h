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
	SpriteShader(const SpriteShader&) = delete;

	UniformLocation_t ProjectionViewLocation;

	UniformLocation_t UseSolidColorLocation;
	UniformLocation_t UseTextShadowLocation;
	UniformLocation_t UseCheckerboardLocation;
	UniformLocation_t CheckerboardSizeLocation;
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
	ComfyShader(const ComfyShader&) = delete;

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
	ScreenShader(const ScreenShader&) = delete;

	UniformLocation_t SaturationLocation;
	UniformLocation_t BrightnessLocation;

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
	LineShader(const LineShader&) = delete;

	UniformLocation_t ModelLocation;
	UniformLocation_t ViewLocation;
	UniformLocation_t ProjectionLocation;

protected:
	virtual void GetAllUniformLocations() override;
	virtual const char* GetVertexShaderPath() override;
	virtual const char* GetFragmentShaderPath() override;
};

// ------------------------------------------------------------------------------------------------

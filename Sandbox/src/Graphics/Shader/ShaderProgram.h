#pragma once
#include "../../pch.h"

typedef GLint UniformLocation_t;
typedef GLuint ShaderID_t;
typedef GLuint ProgramID_t;

enum class ShaderType
{
	Vertex, Fragment
};

class ShaderProgram
{
public:
	ShaderProgram();
	~ShaderProgram();

	void Use();
	void SetUniform(UniformLocation_t, int);
	void SetUniform(UniformLocation_t, float);
	void SetUniform(UniformLocation_t, glm::mat4&);
	void SetUniformByName(const char*, int);
	void SetUniformByName(const char*, float);
	void SetUniformByName(const char*, glm::mat4&);

	void Initialize();
	inline bool GetIsInitialized() const { return initialized; };

protected:
	ProgramID_t programID = NULL;

	UniformLocation_t GetUniformLocation(const std::string&);

	virtual void GetAllUniformLocations() = 0;
	virtual const char* GetVertexShaderPath() = 0;
	virtual const char* GetFragmentShaderPath() = 0;

private:
	bool initialized = false;

	int GetShaderSource(const std::string&, char*, size_t);
	int CompileShader(ShaderType, ShaderID_t*, const std::string&);
	int AttachLinkShaders(ShaderID_t, ShaderID_t);
};

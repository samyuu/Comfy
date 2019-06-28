#include "ShaderProgram.h"
#include "Graphics/ErrorChecking.h"
#include "FileSystem/FileHelper.h"
#include <fstream>

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
	if (programID != NULL)
		glDeleteProgram(programID);
}

void ShaderProgram::Bind() const
{
	glUseProgram(programID);
}

void ShaderProgram::UnBind() const
{
	glUseProgram(0);
}

void ShaderProgram::SetUniform(UniformLocation_t location, int value)
{
	glUniform1i(location, value);
}

void ShaderProgram::SetUniform(UniformLocation_t location, float value)
{
	glUniform1f(location, value);
}

void ShaderProgram::SetUniform(UniformLocation_t location, glm::mat4& value)
{
	glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void ShaderProgram::SetUniformByName(const char* name, int value)
{
	SetUniform(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniformByName(const char* name, float value)
{
	SetUniform(GetUniformLocation(name), value);
}

void ShaderProgram::SetUniformByName(const char* name, glm::mat4& value)
{
	SetUniform(GetUniformLocation(name), value);
}

void ShaderProgram::Initialize()
{
	LoadShaderSources();

	ShaderID_t vertexShader, fragmentShader;
	CompileShader(ShaderType::Vertex, &vertexShader, vertexSource);
	CompileShader(ShaderType::Fragment, &fragmentShader, fragmentSource);

	programID = glCreateProgram();
	CHECK_GL_ERROR("glCreateProgram()");

	AttachLinkShaders(vertexShader, fragmentShader);

	GetAllUniformLocations();
	initialized = true;
}

UniformLocation_t ShaderProgram::GetUniformLocation(const std::string &name)
{
	return glGetUniformLocation(programID, name.c_str());
	CHECK_GL_ERROR("glGetUniformLocation()");
}

void ShaderProgram::LoadShaderSources()
{
	FileSystem::ReadAllBytes(GetVertexShaderPath(), &vertexSource);
	FileSystem::ReadAllBytes(GetFragmentShaderPath(), &fragmentSource);
}

int ShaderProgram::CompileShader(ShaderType shaderType, ShaderID_t* shaderID, const std::vector<uint8_t>& shaderSource)
{
	GLuint glShaderType = NULL;

	switch (shaderType)
	{
	case ShaderType::Vertex:
		glShaderType = GL_VERTEX_SHADER;
		break;
	
	case ShaderType::Fragment:
		glShaderType = GL_FRAGMENT_SHADER;
		break;
	}

	*shaderID = glCreateShader(glShaderType);
	CHECK_GL_ERROR("glCreateShader()");

	int sourceSizes[1] = { (int)shaderSource.size() };
	char* sources[1] = { (char*)shaderSource.data() };
	
	glShaderSource(*shaderID, 1, sources, sourceSizes);
	CHECK_GL_ERROR("glShaderSource()");
	glCompileShader(*shaderID);
	CHECK_GL_ERROR("glCompileShader()");

	int compileSuccess = NULL;
	glGetShaderiv(*shaderID, GL_COMPILE_STATUS, &compileSuccess);

	if (!compileSuccess)
	{
		char infoLog[512] = {};
		glGetShaderInfoLog(*shaderID, sizeof(infoLog), NULL, infoLog);

		Logger::LogErrorLine("ShaderProgram::CompileShader(): Failed to compile shader %s", infoLog);
		return -1;
	}

	return 0;
}

int ShaderProgram::AttachLinkShaders(ShaderID_t vertexShader, ShaderID_t fragmentShader)
{
	glAttachShader(programID, vertexShader);
	CHECK_GL_ERROR("glAttachShader()");
	glAttachShader(programID, fragmentShader);
	CHECK_GL_ERROR("glAttachShader()");
	glLinkProgram(programID);
	CHECK_GL_ERROR("glLinkProgram()");

	int linkSuccess = NULL;
	glGetProgramiv(programID, GL_LINK_STATUS, &linkSuccess);

	if (!linkSuccess)
	{
		char infoLog[512] = {};
		glGetProgramInfoLog(programID, sizeof(infoLog), NULL, infoLog);

		Logger::LogErrorLine("ShaderProgram::AttachLinkShaders(): Failed to link shaders %s", infoLog);
		return -1;
	}

	glDetachShader(programID, vertexShader);
	glDetachShader(programID, fragmentShader);

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return 0;
}

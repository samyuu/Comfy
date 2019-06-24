#include "ShaderProgram.h"
#include <fstream>
#include "../../Logger.h"

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
	if (programID != NULL)
		glDeleteProgram(programID);
}

void ShaderProgram::Use()
{
	glUseProgram(programID);
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
	ShaderID_t vertexShader, fragmentShader;
	char sourceBuffer[4096];

	GetShaderSource(GetVertexShaderPath(), sourceBuffer, sizeof(sourceBuffer));
	CompileShader(ShaderType::Vertex, &vertexShader, sourceBuffer);

	GetShaderSource(GetFragmentShaderPath(), sourceBuffer, sizeof(sourceBuffer));
	CompileShader(ShaderType::Fragment, &fragmentShader, sourceBuffer);

	programID = glCreateProgram();
	AttachLinkShaders(vertexShader, fragmentShader);

	GetAllUniformLocations();
	initialized = true;
}

UniformLocation_t ShaderProgram::GetUniformLocation(const std::string &name)
{
	return glGetUniformLocation(programID, name.c_str());
}

int ShaderProgram::GetShaderSource(const std::string& path, char* buffer, size_t bufferSize)
{
	std::ifstream file(path, std::ifstream::in);

	int i = 0;
	while (file.good())
	{
		buffer[i] = file.get();

		if (!file.eof())
			i++;
	}
	buffer[i] = '\0';

	file.close();
	return 0;
}

int ShaderProgram::CompileShader(ShaderType shaderType, ShaderID_t* shaderID, const std::string& shaderSource)
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

	const char* source = shaderSource.c_str();
	glShaderSource(*shaderID, 1, &source, NULL);
	
	glCompileShader(*shaderID);

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
	glAttachShader(programID, fragmentShader);
	glLinkProgram(programID);

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

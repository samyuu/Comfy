#include "ShaderProgram.h"
#include "FileSystem/FileHelper.h"

ShaderProgram::ShaderProgram()
{
}

ShaderProgram::~ShaderProgram()
{
	Dispose();
}

void ShaderProgram::Bind() const
{
	GLCall(glUseProgram(programID));
}

void ShaderProgram::UnBind() const
{
	GLCall(glUseProgram(0));
}

void ShaderProgram::SetUniform(UniformLocation_t location, int value)
{
	GLCall(glUniform1i(location, value));
}

void ShaderProgram::SetUniform(UniformLocation_t location, float value)
{
	GLCall(glUniform1f(location, value));
}

void ShaderProgram::SetUniform(UniformLocation_t location, glm::mat4& value)
{
	GLCall(glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value)));
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

	GLCall(programID = glCreateProgram());

	AttachLinkShaders(vertexShader, fragmentShader);

	GetAllUniformLocations();
	initialized = true;
}

UniformLocation_t ShaderProgram::GetUniformLocation(const std::string &name)
{
	GLint location;
	GLCall(location = glGetUniformLocation(programID, name.c_str()));
	return location;
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

	int sourceSizes[1] = { (int)shaderSource.size() };
	char* sources[1] = { (char*)shaderSource.data() };
	
	GLCall(glShaderSource(*shaderID, 1, sources, sourceSizes));
	GLCall(glCompileShader(*shaderID));

	int compileSuccess = NULL;
	GLCall(glGetShaderiv(*shaderID, GL_COMPILE_STATUS, &compileSuccess));

	if (!compileSuccess)
	{
		char infoLog[512] = {};
		GLCall(glGetShaderInfoLog(*shaderID, sizeof(infoLog), NULL, infoLog));

		Logger::LogErrorLine(__FUNCTION__"(): Failed to compile shader %s", infoLog);
		return -1;
	}

	return 0;
}

int ShaderProgram::AttachLinkShaders(ShaderID_t vertexShader, ShaderID_t fragmentShader)
{
	GLCall(glAttachShader(programID, vertexShader));
	GLCall(glAttachShader(programID, fragmentShader));
	GLCall(glLinkProgram(programID));

	int linkSuccess = NULL;
	GLCall(glGetProgramiv(programID, GL_LINK_STATUS, &linkSuccess));

	if (!linkSuccess)
	{
		char infoLog[512] = {};
		GLCall(glGetProgramInfoLog(programID, sizeof(infoLog), NULL, infoLog));

		Logger::LogErrorLine(__FUNCTION__"(): Failed to link shaders %s", infoLog);
		return -1;
	}

	GLCall(glDetachShader(programID, vertexShader));
	GLCall(glDetachShader(programID, fragmentShader));

	GLCall(glDeleteShader(vertexShader));
	GLCall(glDeleteShader(fragmentShader));

	return 0;
}

void ShaderProgram::Dispose()
{
	if (programID != NULL)
	{
		GLCall(glDeleteProgram(programID));
		programID = NULL;
	}
}

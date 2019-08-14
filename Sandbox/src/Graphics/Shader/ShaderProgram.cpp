#include "ShaderProgram.h"
#include "FileSystem/FileHelper.h"
#include <algorithm>
#include <assert.h>

namespace Graphics
{
	constexpr int32_t UniformLocation_Uninitialized = 0xCCCCCCCC;

	Uniform::Uniform(UniformType type, const char* name) : location(UniformLocation_Uninitialized), type(type), name(name)
	{
	}

	void Uniform::UpdateLocation(const ShaderProgram& shader)
	{
		GLCall(location = glGetUniformLocation(shader.GetProgramID(), name));

		if (location == -1)
			Logger::LogLine(__FUNCTION__ "(): %s not found", GetName());
	}

	void Uniform::AssertLocationSetType(UniformType targetType) const
	{
		assert(location != UniformLocation_Uninitialized);
		assert(type == targetType);
	}

	int32_t Uniform::GetLocation() const
	{
		return location;
	}

	UniformType Uniform::GetType() const
	{
		return type;
	}

	const char* Uniform::GetName() const
	{
		return name;
	}

	ShaderProgram::ShaderProgram()
	{
		RegisterProgram(this);
	}

	ShaderProgram::~ShaderProgram()
	{
		UnregisterProgram(this);
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

	void ShaderProgram::SetUniform(const Uniform& uniform, int value)
	{
		uniform.AssertLocationSetType(UniformType::Int);
		GLCall(glUniform1i(uniform.GetLocation(), value));
	}

	void ShaderProgram::SetUniform(const Uniform& uniform, float value)
	{
		uniform.AssertLocationSetType(UniformType::Float);
		GLCall(glUniform1f(uniform.GetLocation(), value));
	}

	void ShaderProgram::SetUniform(const Uniform& uniform, const vec2& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec2);
		GLCall(glUniform2f(uniform.GetLocation(), value.x, value.y));
	}

	void ShaderProgram::SetUniform(const Uniform& uniform, const vec3& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec3);
		GLCall(glUniform3f(uniform.GetLocation(), value.x, value.y, value.z));
	}

	void ShaderProgram::SetUniform(const Uniform& uniform, const vec4& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec4);
		GLCall(glUniform4f(uniform.GetLocation(), value.x, value.y, value.z, value.w));
	}

	void ShaderProgram::SetUniform(const Uniform& uniform, const glm::mat4& value)
	{
		uniform.AssertLocationSetType(UniformType::Mat4);
		GLCall(glUniformMatrix4fv(uniform.GetLocation(), 1, GL_FALSE, glm::value_ptr(value)));
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

	void ShaderProgram::Recompile()
	{
		if (!GetIsInitialized())
			return;

		Dispose();
		Initialize();
	}

	void ShaderProgram::RecompileAllShaders()
	{
		Logger::LogLine(__FUNCTION__ "(): Recompiling Shaders...");

		for (auto shader : allShaderPrograms)
		{
			shader->Recompile();
		}
	}

	const std::vector<ShaderProgram*>& ShaderProgram::GetAllShaderPrograms()
	{
		return allShaderPrograms;
	}

	void ShaderProgram::UpdateUniformLocation(Uniform& uniform) const
	{
		uniform.UpdateLocation(*this);
	}

	void ShaderProgram::UpdateUniformArrayLocations(Uniform* firstUniform, Uniform* lastUniform) const
	{
		for (Uniform* uniform = firstUniform; uniform <= lastUniform; uniform++)
			uniform->UpdateLocation(*this);
	}

	void ShaderProgram::GetAllUniformLocations()
	{
		UpdateUniformArrayLocations(GetFirstUniform(), GetLastUniform());
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

		int sourceSizes[1] = { static_cast<int>(shaderSource.size()) };
		const char* sources[1] = { reinterpret_cast<const char*>(shaderSource.data()) };

		GLCall(glShaderSource(*shaderID, 1, sources, sourceSizes));
		GLCall(glCompileShader(*shaderID));

		int compileSuccess = NULL;
		GLCall(glGetShaderiv(*shaderID, GL_COMPILE_STATUS, &compileSuccess));

		if (!compileSuccess)
		{
			std::string infoLog;
			ReserveShaderInfoLogLength(*shaderID, infoLog);
			GLCall(glGetShaderInfoLog(*shaderID, static_cast<int>(infoLog.capacity()), NULL, infoLog.data()));

			Logger::LogErrorLine(__FUNCTION__"(): Failed to compile shader %s", infoLog.c_str());
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
			std::string infoLog;
			ReserveProgramInfoLogLength(programID, infoLog);
			GLCall(glGetProgramInfoLog(programID, static_cast<int>(infoLog.capacity()), NULL, infoLog.data()));

			Logger::LogErrorLine(__FUNCTION__"(): Failed to link shaders %s", infoLog.c_str());
		}

		GLCall(glDetachShader(programID, vertexShader));
		GLCall(glDetachShader(programID, fragmentShader));

		GLCall(glDeleteShader(vertexShader));
		GLCall(glDeleteShader(fragmentShader));

		return linkSuccess;
	}

	void ShaderProgram::Dispose()
	{
		if (programID != NULL)
		{
			GLCall(glDeleteProgram(programID));
			programID = NULL;
		}
	}

	void ShaderProgram::ReserveShaderInfoLogLength(const ShaderID_t& shaderID, std::string& infoLog)
	{
		int logLength;
		GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength));

		if (logLength > 0)
			infoLog.reserve(logLength);
	}

	void ShaderProgram::ReserveProgramInfoLogLength(const ProgramID_t& programID, std::string & infoLog)
	{
		int logLength;
		GLCall(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength));

		if (logLength > 0)
			infoLog.reserve(logLength);
	}

	std::vector<ShaderProgram*> ShaderProgram::allShaderPrograms;

	void ShaderProgram::RegisterProgram(ShaderProgram* program)
	{
		allShaderPrograms.push_back(program);
	}

	void ShaderProgram::UnregisterProgram(ShaderProgram* program)
	{
		allShaderPrograms.erase(std::remove(allShaderPrograms.begin(), allShaderPrograms.end(), program), allShaderPrograms.end());
	}
}

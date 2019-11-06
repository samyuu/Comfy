#include "GL_ShaderProgram.h"
#include "Graphics/RenderCommand.h"
#include "Core/ComfyData.h"
#include <algorithm>
#include <assert.h>

namespace Graphics
{
	constexpr int32_t UniformLocation_Uninitialized = 0xCCCCCCCC;

	GL_Uniform::GL_Uniform(UniformType type, const char* name) : location(UniformLocation_Uninitialized), type(type), name(name)
	{
	}

	void GL_Uniform::UpdateLocation(const GL_ShaderProgram& shader)
	{
		GLCall(location = glGetUniformLocation(shader.GetProgramID(), name));

		if (location == -1)
			Logger::LogLine(__FUNCTION__ "(): %s not found", GetName());
	}

	void GL_Uniform::AssertLocationSetType(UniformType targetType) const
	{
		assert(location != UniformLocation_Uninitialized);
		assert(type == targetType);
	}

	int32_t GL_Uniform::GetLocation() const
	{
		return location;
	}

	UniformType GL_Uniform::GetType() const
	{
		return type;
	}

	const char* GL_Uniform::GetName() const
	{
		return name;
	}

	GL_ShaderProgram::GL_ShaderProgram()
	{
		RegisterProgram(this);
	}

	GL_ShaderProgram::~GL_ShaderProgram()
	{
		UnregisterProgram(this);
		Dispose();
	}

	void GL_ShaderProgram::Bind() const
	{
		RenderCommand::BindShaderProgram(programID);
	}

	void GL_ShaderProgram::UnBind() const
	{
		RenderCommand::BindShaderProgram(0);
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, int value)
	{
		uniform.AssertLocationSetType(UniformType::Int);
		GLCall(glUniform1i(uniform.GetLocation(), value));
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, float value)
	{
		uniform.AssertLocationSetType(UniformType::Float);
		GLCall(glUniform1f(uniform.GetLocation(), value));
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, const vec2& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec2);
		GLCall(glUniform2f(uniform.GetLocation(), value.x, value.y));
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, const vec3& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec3);
		GLCall(glUniform3f(uniform.GetLocation(), value.x, value.y, value.z));
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, const vec4& value)
	{
		uniform.AssertLocationSetType(UniformType::Vec4);
		GLCall(glUniform4f(uniform.GetLocation(), value.x, value.y, value.z, value.w));
	}

	void GL_ShaderProgram::SetUniform(const GL_Uniform& uniform, const glm::mat4& value)
	{
		uniform.AssertLocationSetType(UniformType::Mat4);
		GLCall(glUniformMatrix4fv(uniform.GetLocation(), 1, GL_FALSE, glm::value_ptr(value)));
	}

	void GL_ShaderProgram::Initialize()
	{
		LoadShaderSources();

		ShaderID_t vertexShader, fragmentShader;
		CompileShader(ShaderType::Vertex, &vertexShader, vertexSource);
		CompileShader(ShaderType::Fragment, &fragmentShader, fragmentSource);

		GLCall(programID = glCreateProgram());

		AttachLinkShaders(vertexShader, fragmentShader);

		SetObjectLabel(GetShaderName());
		GetAllUniformLocations();
		
		initialized = true;
	}

	void GL_ShaderProgram::SetObjectLabel(const char* label)
	{
		GLCall(glObjectLabel(GL_PROGRAM, programID, -1, label));
	}

	void GL_ShaderProgram::Recompile()
	{
		if (!GetIsInitialized())
			return;

		Dispose();
		Initialize();
	}

	void GL_ShaderProgram::RecompileAllShaders()
	{
		Logger::LogLine(__FUNCTION__ "(): Recompiling Shaders...");

		for (auto shader : allShaderPrograms)
		{
			shader->Recompile();
		}
	}

	const std::vector<GL_ShaderProgram*>& GL_ShaderProgram::GetAllShaderPrograms()
	{
		return allShaderPrograms;
	}

	void GL_ShaderProgram::UpdateUniformLocation(GL_Uniform& uniform) const
	{
		uniform.UpdateLocation(*this);
	}

	void GL_ShaderProgram::UpdateUniformArrayLocations(GL_Uniform* firstUniform, GL_Uniform* lastUniform) const
	{
		for (GL_Uniform* uniform = firstUniform; uniform <= lastUniform; uniform++)
			uniform->UpdateLocation(*this);
	}

	void GL_ShaderProgram::GetAllUniformLocations()
	{
		UpdateUniformArrayLocations(GetFirstUniform(), GetLastUniform());
	}

	void GL_ShaderProgram::LoadShaderSources()
	{
		// TODO: Optionally read from file directly for debug builds (?)

		bool success;
		success = ComfyData->ReadFileIntoBuffer(GetVertexShaderPath(), vertexSource);
		assert(success);

		success = ComfyData->ReadFileIntoBuffer(GetFragmentShaderPath(), fragmentSource);
		assert(success);
	}

	int GL_ShaderProgram::CompileShader(ShaderType shaderType, ShaderID_t* shaderID, const std::vector<uint8_t>& shaderSource)
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

	int GL_ShaderProgram::AttachLinkShaders(ShaderID_t vertexShader, ShaderID_t fragmentShader)
	{
		GLCall(glAttachShader(programID, vertexShader));
		GLCall(glObjectLabel(GL_SHADER, vertexShader, -1, "ShaderProgram::VertexShader"));

		GLCall(glAttachShader(programID, fragmentShader));
		GLCall(glObjectLabel(GL_SHADER, fragmentShader, -1, "ShaderProgram::FragmentShader"));
		
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

	void GL_ShaderProgram::Dispose()
	{
		if (programID != NULL)
		{
			GLCall(glDeleteProgram(programID));
			programID = NULL;
		}
	}

	void GL_ShaderProgram::ReserveShaderInfoLogLength(const ShaderID_t& shaderID, std::string& infoLog)
	{
		int logLength;
		GLCall(glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &logLength));

		if (logLength > 0)
			infoLog.reserve(logLength);
	}

	void GL_ShaderProgram::ReserveProgramInfoLogLength(const ProgramID_t& programID, std::string & infoLog)
	{
		int logLength;
		GLCall(glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &logLength));

		if (logLength > 0)
			infoLog.reserve(logLength);
	}

	std::vector<GL_ShaderProgram*> GL_ShaderProgram::allShaderPrograms;

	void GL_ShaderProgram::RegisterProgram(GL_ShaderProgram* program)
	{
		allShaderPrograms.push_back(program);
	}

	void GL_ShaderProgram::UnregisterProgram(GL_ShaderProgram* program)
	{
		allShaderPrograms.erase(std::remove(allShaderPrograms.begin(), allShaderPrograms.end(), program), allShaderPrograms.end());
	}
}

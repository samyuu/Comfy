#pragma once
#include "Types.h"
#include "Graphics/GraphicTypes.h"
#include "Graphics/OpenGL/OpenGL.h"
#include "Graphics/GraphicsInterface.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	typedef GLuint ShaderID_t;
	typedef GLuint ProgramID_t;

	class GL_ShaderProgram;

	enum class ShaderType
	{
		Vertex, Fragment
	};

	enum class UniformType
	{
		Int, Float, Vec2, Vec3, Vec4, Mat4, Count
	};

	class GL_Uniform
	{
	public:
		GL_Uniform(UniformType type, const char* name);

		void UpdateLocation(const GL_ShaderProgram& shader);
		void AssertLocationSetType(UniformType targetType) const;
		int32_t GetLocation() const;
		UniformType GetType() const;
		const char* GetName() const;

	private:
		int32_t location;
		UniformType type;
		const char* name;
	};

	class GL_ShaderProgram : public IBindable, ILabeledObject
	{
	public:
		GL_ShaderProgram();
		GL_ShaderProgram(const GL_ShaderProgram&) = delete;
		GL_ShaderProgram& operator= (const GL_ShaderProgram&) = delete;
		virtual ~GL_ShaderProgram();

		void Bind() const override;
		void UnBind() const override;
		void SetUniform(const GL_Uniform&, int);
		void SetUniform(const GL_Uniform&, float);
		void SetUniform(const GL_Uniform&, const vec2&);
		void SetUniform(const GL_Uniform&, const vec3&);
		void SetUniform(const GL_Uniform&, const vec4&);
		void SetUniform(const GL_Uniform&, const glm::mat4&);

		void Initialize();
		inline bool GetIsInitialized() const { return initialized; };
		inline ProgramID_t GetProgramID() const { return programID; };
		
		void SetObjectLabel(const char* label) override;

	public:
		void Recompile();
		static void RecompileAllShaders();

		virtual GL_Uniform* GetFirstUniform() = 0;
		virtual GL_Uniform* GetLastUniform() = 0;

		virtual const char* GetShaderName() = 0;
		virtual const char* GetVertexShaderPath() = 0;
		virtual const char* GetFragmentShaderPath() = 0;

		static const std::vector<GL_ShaderProgram*>& GetAllShaderPrograms();

	protected:
		std::vector<uint8_t> vertexSource, fragmentSource;
		ProgramID_t programID = NULL;

		void UpdateUniformLocation(GL_Uniform& uniform) const;
		void UpdateUniformArrayLocations(GL_Uniform* firstUniform, GL_Uniform* lastUniform) const;

		virtual void GetAllUniformLocations();

	private:
		bool initialized = false;

		void LoadShaderSources();
		int CompileShader(ShaderType, ShaderID_t*, const std::vector<uint8_t>&);
		int AttachLinkShaders(ShaderID_t, ShaderID_t);
		void Dispose();

		static void ReserveShaderInfoLogLength(const ShaderID_t& shaderID, std::string& infoLog);
		static void ReserveProgramInfoLogLength(const ProgramID_t& programID, std::string& infoLog);

	private:
		static std::vector<GL_ShaderProgram*> allShaderPrograms;

		static void RegisterProgram(GL_ShaderProgram* program);
		static void UnregisterProgram(GL_ShaderProgram* program);
	};
}

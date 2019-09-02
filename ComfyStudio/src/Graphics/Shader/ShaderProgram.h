#pragma once
#include "Types.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsInterface.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
	typedef GLuint ShaderID_t;
	typedef GLuint ProgramID_t;

	class ShaderProgram;

	enum class ShaderType
	{
		Vertex, Fragment
	};

	enum class UniformType
	{
		Int, Float, Vec2, Vec3, Vec4, Mat4, Count
	};

	class Uniform
	{
	public:
		Uniform(UniformType type, const char* name);

		void UpdateLocation(const ShaderProgram& shader);
		void AssertLocationSetType(UniformType targetType) const;
		int32_t GetLocation() const;
		UniformType GetType() const;
		const char* GetName() const;

	private:
		int32_t location;
		UniformType type;
		const char* name;
	};

	class ShaderProgram : public IBindable
	{
	public:
		ShaderProgram();
		ShaderProgram(const ShaderProgram&) = delete;
		virtual ~ShaderProgram();

		void Bind() const override;
		void UnBind() const override;
		void SetUniform(const Uniform&, int);
		void SetUniform(const Uniform&, float);
		void SetUniform(const Uniform&, const vec2&);
		void SetUniform(const Uniform&, const vec3&);
		void SetUniform(const Uniform&, const vec4&);
		void SetUniform(const Uniform&, const glm::mat4&);

		void Initialize();
		inline bool GetIsInitialized() const { return initialized; };
		inline ProgramID_t GetProgramID() const { return programID; };

	public:
		void Recompile();
		static void RecompileAllShaders();

		virtual Uniform* GetFirstUniform() = 0;
		virtual Uniform* GetLastUniform() = 0;

		virtual const char* GetShaderName() = 0;
		virtual const char* GetVertexShaderPath() = 0;
		virtual const char* GetFragmentShaderPath() = 0;

		static const Vector<ShaderProgram*>& GetAllShaderPrograms();

	protected:
		Vector<uint8_t> vertexSource, fragmentSource;
		ProgramID_t programID = NULL;

		void UpdateUniformLocation(Uniform& uniform) const;
		void UpdateUniformArrayLocations(Uniform* firstUniform, Uniform* lastUniform) const;

		virtual void GetAllUniformLocations();

	private:
		bool initialized = false;

		void LoadShaderSources();
		int CompileShader(ShaderType, ShaderID_t*, const Vector<uint8_t>&);
		int AttachLinkShaders(ShaderID_t, ShaderID_t);
		void Dispose();

		static void ReserveShaderInfoLogLength(const ShaderID_t& shaderID, String& infoLog);
		static void ReserveProgramInfoLogLength(const ProgramID_t& programID, String& infoLog);

	private:
		static Vector<ShaderProgram*> allShaderPrograms;

		static void RegisterProgram(ShaderProgram* program);
		static void UnregisterProgram(ShaderProgram* program);
	};
}

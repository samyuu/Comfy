#pragma once
#include "Types.h"
#include "Graphics/Graphics.h"
#include "Graphics/GraphicsInterface.h"
#include <string>
#include <vector>

typedef GLint UniformLocation_t;
typedef GLuint ShaderID_t;
typedef GLuint ProgramID_t;

enum class ShaderType
{
	Vertex, Fragment
};

class ShaderProgram : public IBindable
{
public:
	ShaderProgram();
	virtual ~ShaderProgram();
	ShaderProgram(const ShaderProgram&) = delete;

	void Bind() const override;
	void UnBind() const override;
	void SetUniform(UniformLocation_t, int);
	void SetUniform(UniformLocation_t, float);
	void SetUniform(UniformLocation_t, glm::mat4&);
	void SetUniformByName(const char*, int);
	void SetUniformByName(const char*, float);
	void SetUniformByName(const char*, glm::mat4&);

	void Initialize();
	inline bool GetIsInitialized() const { return initialized; };

protected:
	std::vector<uint8_t> vertexSource, fragmentSource;
	ProgramID_t programID = NULL;

	UniformLocation_t GetUniformLocation(const std::string&);

	virtual void GetAllUniformLocations() = 0;
	virtual const char* GetVertexShaderPath() = 0;
	virtual const char* GetFragmentShaderPath() = 0;

private:
	bool initialized = false;

	void LoadShaderSources();
	int CompileShader(ShaderType, ShaderID_t*, const std::vector<uint8_t>&);
	int AttachLinkShaders(ShaderID_t, ShaderID_t);
	void Dispose();
};

#pragma once
#include "../pch.h"

// ------------------------------------------------------------------------------------------------
// --- VertexBuffer:
// ------------------------------------------------------------------------------------------------

typedef GLuint VertexBufferID_t;

enum class BufferUsage
{
	StreamDraw,
	StreamRead,
	StreamCopy,
	StaticDraw,
	StaticRead,
	StaticCopy,
	DynamicDraw,
	DynamicRead,
	DynamicCopy,
};

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();
	VertexBuffer(const VertexBuffer&) = delete;

	void Initialize();
	void BufferData(void* data, size_t dataSize, BufferUsage usage);

	void Bind();
	void UnBind();

protected:
	VertexBufferID_t vertexBufferID = NULL;
	BufferUsage bufferUsage;
	
	GLenum GetGLUsage() const;
	GLenum GetGLBufferTarget() const;
	void Dispose();
};

// ------------------------------------------------------------------------------------------------
// --- IndexBuffer:
// ------------------------------------------------------------------------------------------------

// TODO: 
// class IndexBuffer
// {
// 
// };

// ------------------------------------------------------------------------------------------------
// --- ShaderDataType:
// ------------------------------------------------------------------------------------------------

enum class ShaderDataType : uint16_t
{
	Int, 
	Float, 
	Bool, 
	Vec2, 
	Vec3, 
	Vec4, 
	Mat3, 
	Mat4,
};

// ------------------------------------------------------------------------------------------------
// --- BufferElement:
// ------------------------------------------------------------------------------------------------

struct BufferElement
{
	const char* Name;
	ShaderDataType Type;
	uint16_t Size;
	uint16_t Offset;
	bool Normalized;

	BufferElement(ShaderDataType type, const char* name);
	
	size_t GetElementCount() const;
	GLenum GetDataType() const;
	bool GetIsNormalized() const;
	inline void* GetOffset() const { return (void*)Offset; };
};

// ------------------------------------------------------------------------------------------------
// --- BufferLayout:
// ------------------------------------------------------------------------------------------------

class BufferLayout
{
public:
	BufferLayout(std::initializer_list<BufferElement> elements);

	inline uint16_t GetStride() const { return stride; };
	inline const std::vector<BufferElement>& GetElemenets() const { return elements; };

private:
	uint16_t stride;
	std::vector<BufferElement> elements;

	void UpdateElements();
	static uint16_t GetElementSize(ShaderDataType type);
};

// ------------------------------------------------------------------------------------------------

#pragma once
#include "../pch.h"

typedef GLuint VertexBufferID_t;
typedef GLenum BufferTarget_t;
typedef GLenum BufferUsage_t;

class VertexBuffer
{
public:
	VertexBuffer();
	~VertexBuffer();
	VertexBuffer(const VertexBuffer&) = delete;

	void Initialize();
	void BufferData(void* data, size_t dataSize, BufferUsage_t usage);

	void Bind();
	void UnBind();

protected:
	VertexBufferID_t vertexBufferID = NULL;

	inline BufferTarget_t GetBufferTarget() { return GL_ARRAY_BUFFER; };
	void Dispose();
};

// TODO: 
// class IndexBuffer
// {
// 
// };
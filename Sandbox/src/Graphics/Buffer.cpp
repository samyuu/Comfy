#include "Buffer.h"

VertexBuffer::VertexBuffer()
{
}

VertexBuffer::~VertexBuffer()
{
	Dispose();
}

void VertexBuffer::Initialize()
{
	glGenBuffers(1, &vertexBufferID);
}

void VertexBuffer::BufferData(void* data, size_t dataSize, BufferUsage_t usage)
{
	glBufferData(GetBufferTarget(), dataSize, data, usage);
}

void VertexBuffer::Bind()
{
	glBindBuffer(GetBufferTarget(), vertexBufferID);
}

void VertexBuffer::UnBind()
{
	glBindBuffer(GetBufferTarget(), NULL);
}

void VertexBuffer::Dispose()
{
	if (vertexBufferID != NULL)
		glDeleteBuffers(1, &vertexBufferID);
}

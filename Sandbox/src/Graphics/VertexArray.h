#pragma once
#include "../pch.h"
class BufferLayout;

typedef GLuint VertexArrayID_t;

class VertexArray
{
public:
	VertexArray();
	~VertexArray();
	VertexArray(const VertexArray&) = delete;

	void Initialize();

	void Bind();
	void UnBind();

	void SetLayout(const BufferLayout& layout);

protected:
	VertexArrayID_t vertexArrayID = NULL;

	void Dispose();
};


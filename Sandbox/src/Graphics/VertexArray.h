#pragma once
#include "../pch.h"

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

protected:
	VertexArrayID_t vertexArrayID = NULL;

	void Dispose();
};


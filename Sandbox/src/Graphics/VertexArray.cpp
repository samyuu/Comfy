#include "VertexArray.h"

VertexArray::VertexArray()
{
}

VertexArray::~VertexArray()
{
	Dispose();
}

void VertexArray::Initialize()
{
	glGenVertexArrays(1, &vertexArrayID);
}

void VertexArray::Bind()
{
	glBindVertexArray(vertexArrayID);
}

void VertexArray::UnBind()
{
	glBindVertexArray(NULL);
}

void VertexArray::Dispose()
{
	if (vertexArrayID != NULL)
		glDeleteVertexArrays(1, &vertexArrayID);
}
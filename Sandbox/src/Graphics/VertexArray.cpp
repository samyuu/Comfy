#include "VertexArray.h"
#include "Buffer.h"

VertexArray::VertexArray()
{
}

VertexArray::~VertexArray()
{
	Dispose();
}

void VertexArray::InitializeID()
{
	GLCall(glGenVertexArrays(1, &vertexArrayID));
}

void VertexArray::Bind() const
{
	GLCall(glBindVertexArray(vertexArrayID));
}

void VertexArray::UnBind() const
{
	GLCall(glBindVertexArray(NULL));
}

void VertexArray::SetLayout(const BufferLayout& layout)
{
	const auto& elements = layout.GetElemenets();

	for (GLuint i = 0; i < static_cast<GLuint>(elements.size()); i++)
	{
		const BufferElement& element = elements[i];
		
		GLCall(glEnableVertexAttribArray(i));
		GLCall(glVertexAttribPointer(i, element.GetElementCount(), element.GetDataType(), element.GetIsNormalized(), layout.GetStride(), element.GetOffset()));
	}
}

void VertexArray::Dispose()
{
	if (vertexArrayID != NULL)
	{
		GLCall(glDeleteVertexArrays(1, &vertexArrayID));
		vertexArrayID = NULL;
	}
}
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

void VertexArray::SetLayout(const BufferLayout& layout)
{
	const auto& elements = layout.GetElemenets();

	for (size_t i = 0; i < elements.size(); i++)
	{
		const auto& element = elements[i];
		
		glEnableVertexAttribArray(i);
		glVertexAttribPointer(i, element.GetElementCount(), element.GetDataType(), element.GetIsNormalized(), layout.GetStride(), element.GetOffset());
	}
}

void VertexArray::Dispose()
{
	if (vertexArrayID != NULL)
		glDeleteVertexArrays(1, &vertexArrayID);
}
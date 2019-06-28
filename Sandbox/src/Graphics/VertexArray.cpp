#include "VertexArray.h"
#include "Buffer.h"
#include "ErrorChecking.h"

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
	CHECK_GL_ERROR("glGenVertexArrays()");
}

void VertexArray::Bind() const
{
	glBindVertexArray(vertexArrayID);
}

void VertexArray::UnBind() const
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
		CHECK_GL_ERROR("glEnableVertexAttribArray()");

		glVertexAttribPointer(i, element.GetElementCount(), element.GetDataType(), element.GetIsNormalized(), layout.GetStride(), element.GetOffset());
		CHECK_GL_ERROR("glVertexAttribPointer()");
	}
}

void VertexArray::Dispose()
{
	if (vertexArrayID != NULL)
		glDeleteVertexArrays(1, &vertexArrayID);
}
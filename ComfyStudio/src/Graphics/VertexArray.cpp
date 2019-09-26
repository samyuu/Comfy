#include "VertexArray.h"
#include "Buffer.h"

namespace Graphics
{
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

	void VertexArray::SetLayout(const BufferLayout& layout, bool interleaved)
	{
		const auto& elements = layout.GetElemenets();

		for (GLuint i = 0; i < static_cast<GLuint>(elements.size()); i++)
		{
			const BufferElement& element = elements[i];
			const VertexBuffer* buffer = element.GetBuffer();

			if (!interleaved && buffer != nullptr)
				buffer->Bind();

			GLCall(glEnableVertexAttribArray(i));
			GLCall(glVertexAttribPointer(i, element.GetElementCount(), element.GetDataType(), element.GetIsNormalized(), interleaved ? layout.GetStride() : element.Size, element.GetOffset()));
		}
	}

	void VertexArray::SetObjectLabel(const char* label)
	{
		GLCall(glObjectLabel(GL_VERTEX_ARRAY, vertexArrayID, -1, label));
	}

	void VertexArray::Dispose()
	{
		if (vertexArrayID != NULL)
		{
			GLCall(glDeleteVertexArrays(1, &vertexArrayID));
			vertexArrayID = NULL;
		}
	}
}

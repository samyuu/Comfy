#include "GL_VertexArray.h"
#include "GL_Buffer.h"

namespace Graphics
{
	GL_VertexArray::GL_VertexArray()
	{
	}

	GL_VertexArray::~GL_VertexArray()
	{
		Dispose();
	}

	void GL_VertexArray::InitializeID()
	{
		GLCall(glGenVertexArrays(1, &vertexArrayID));
	}

	void GL_VertexArray::Bind() const
	{
		GLCall(glBindVertexArray(vertexArrayID));
	}

	void GL_VertexArray::UnBind() const
	{
		GLCall(glBindVertexArray(NULL));
	}

	void GL_VertexArray::SetLayout(const BufferLayout& layout, bool interleaved)
	{
		const auto& elements = layout.GetElemenets();

		for (GLuint i = 0; i < static_cast<GLuint>(elements.size()); i++)
		{
			const BufferElement& element = elements[i];
			const GL_VertexBuffer* buffer = element.GetBuffer();

			if (!interleaved && buffer != nullptr)
				buffer->Bind();

			GLCall(glEnableVertexAttribArray(i));
			GLCall(glVertexAttribPointer(i, element.GetElementCount(), element.GetDataType(), element.GetIsNormalized(), interleaved ? layout.GetStride() : element.Size, element.GetOffset()));
		}
	}

	void GL_VertexArray::SetObjectLabel(const char* label)
	{
		GLCall(glObjectLabel(GL_VERTEX_ARRAY, vertexArrayID, -1, label));
	}

	void GL_VertexArray::Dispose()
	{
		if (vertexArrayID != NULL)
		{
			GLCall(glDeleteVertexArrays(1, &vertexArrayID));
			vertexArrayID = NULL;
		}
	}
}

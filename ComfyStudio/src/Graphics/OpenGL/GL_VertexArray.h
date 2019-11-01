#pragma once
#include "OpenGL.h"
#include "Graphics/GraphicsInterface.h"

namespace Graphics
{
	class BufferLayout;

	typedef GLuint VertexArrayID_t;

	class GL_VertexArray : public IGraphicsObject
	{
	public:
		GL_VertexArray();
		GL_VertexArray(const GL_VertexArray&) = delete;
		GL_VertexArray& operator= (const GL_VertexArray&) = delete;
		~GL_VertexArray();

		void InitializeID() override;
		void Bind() const override;
		void UnBind() const override;

		void SetLayout(const BufferLayout& layout, bool interleaved = true);
		void SetObjectLabel(const char* label) override;

	protected:
		VertexArrayID_t vertexArrayID = NULL;

		void Dispose();
	};
}

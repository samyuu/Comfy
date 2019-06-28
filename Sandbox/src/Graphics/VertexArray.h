#pragma once
#include "Types.h"
#include "GraphicsInterface.h"

class BufferLayout;

typedef uint32_t VertexArrayID_t;

class VertexArray : public IGraphicsObject
{
public:
	VertexArray();
	~VertexArray();
	VertexArray(const VertexArray&) = delete;

	void InitializeID() override;
	void Bind() const override;
	void UnBind() const override;

	void SetLayout(const BufferLayout& layout);

protected:
	VertexArrayID_t vertexArrayID = NULL;

	void Dispose();
};


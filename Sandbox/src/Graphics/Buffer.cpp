#include "Buffer.h"
#include <assert.h>

// ------------------------------------------------------------------------------------------------
// --- VertexBuffer:
// ------------------------------------------------------------------------------------------------

VertexBuffer::VertexBuffer()
{
}

VertexBuffer::~VertexBuffer()
{
	Dispose();
}

void VertexBuffer::InitializeID()
{
	glGenBuffers(1, &vertexBufferID);
}

void VertexBuffer::Upload(void* data, size_t dataSize, BufferUsage usage)
{
	bufferUsage = usage;
	glBufferData(GetGLBufferTarget(), dataSize, data, GetGLUsage());
}

void VertexBuffer::Bind()
{
	glBindBuffer(GetGLBufferTarget(), vertexBufferID);
}

void VertexBuffer::UnBind()
{
	glBindBuffer(GetGLBufferTarget(), NULL);
}

GLenum VertexBuffer::GetGLUsage() const
{
	switch (bufferUsage)
	{
	case BufferUsage::StreamDraw:
		return GL_STREAM_DRAW;
	case BufferUsage::StreamRead:
		return GL_STREAM_READ;
	case BufferUsage::StreamCopy:
		return GL_STREAM_COPY;
	case BufferUsage::StaticDraw:
		return GL_STATIC_DRAW;
	case BufferUsage::StaticRead:
		return GL_STATIC_READ;
	case BufferUsage::StaticCopy:
		return GL_STATIC_COPY;
	case BufferUsage::DynamicDraw:
		return GL_DYNAMIC_DRAW;
	case BufferUsage::DynamicRead:
		return GL_DYNAMIC_READ;
	case BufferUsage::DynamicCopy:
		return GL_DYNAMIC_COPY;
	default:
		assert(false);
	}
}

GLenum VertexBuffer::GetGLBufferTarget() const
{
	return GL_ARRAY_BUFFER;
}

void VertexBuffer::Dispose()
{
	if (vertexBufferID != NULL)
		glDeleteBuffers(1, &vertexBufferID);
}

// ------------------------------------------------------------------------------------------------
// --- BufferElement:
// ------------------------------------------------------------------------------------------------

BufferElement::BufferElement(ShaderDataType type, const char* name) : Name(name), Type(type), Size(0), Offset(0), Normalized(false)
{
}

size_t BufferElement::GetElementCount() const
{
	switch (Type)
	{
	case ShaderDataType::Int:
	case ShaderDataType::Float:
	case ShaderDataType::Bool:
		return 1;
	case ShaderDataType::Vec2:
		return vec2::length();
	case ShaderDataType::Vec3:
		return vec3::length();
	case ShaderDataType::Vec4:
		return vec4::length();
	case ShaderDataType::Mat3:
		return mat3::length();
	case ShaderDataType::Mat4:
		return mat4::length();
	default:
		assert(false);
	}
}

GLenum BufferElement::GetDataType() const
{
	switch (Type)
	{
	case ShaderDataType::Int:
		return GL_INT;
	case ShaderDataType::Float:
		return GL_FLOAT;
	case ShaderDataType::Bool:
		return GL_BOOL;
	case ShaderDataType::Vec2:
	case ShaderDataType::Vec3:
	case ShaderDataType::Vec4:
	case ShaderDataType::Mat3:
	case ShaderDataType::Mat4:
		return GL_FLOAT;
	default:
		assert(false);
	}
}

bool BufferElement::GetIsNormalized() const
{
	return Normalized;
}

// ------------------------------------------------------------------------------------------------
// --- BufferLayout:
// ------------------------------------------------------------------------------------------------

BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements) : elements(elements)
{
	UpdateElements();
}

void BufferLayout::UpdateElements()
{
	for (BufferElement& element : elements)
	{
		element.Size = GetElementSize(element.Type);
		element.Offset = stride;
		
		stride += element.Size;
	}
}

uint16_t BufferLayout::GetElementSize(ShaderDataType type)
{
	switch (type)
	{
	case ShaderDataType::Int:
		return sizeof(int);
	case ShaderDataType::Float:
		return sizeof(float);
	case ShaderDataType::Bool:
		return sizeof(bool);
	case ShaderDataType::Vec2:
		return sizeof(vec2);
	case ShaderDataType::Vec3:
		return sizeof(vec3);
	case ShaderDataType::Vec4:
		return sizeof(vec4);
	case ShaderDataType::Mat3:
		return sizeof(mat3);
	case ShaderDataType::Mat4:
		return sizeof(mat4);
	default:
		assert(false);
	}
}

// ------------------------------------------------------------------------------------------------

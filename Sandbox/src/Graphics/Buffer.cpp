#include "Buffer.h"
#include <assert.h>

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- Buffer:
	// ------------------------------------------------------------------------------------------------

	Buffer::Buffer(BufferUsage usage) : bufferUsage(usage)
	{
	}

	Buffer::~Buffer()
	{
		Dispose();
	}

	void Buffer::InitializeID()
	{
		GLCall(glGenBuffers(1, &bufferID));
	}

	void Buffer::Upload(size_t dataSize, void* data)
	{
		GLCall(glBufferData(GetGLBufferTarget(), dataSize, data, GetGLUsage()));
	}

	void Buffer::UploadSubData(size_t dataSize, size_t* offset, void* data)
	{
		GLCall(glBufferSubData(GetGLBufferTarget(), (GLintptr)offset, dataSize, data));
	}

	void Buffer::Bind() const
	{
		GLCall(glBindBuffer(GetGLBufferTarget(), bufferID));
	}

	void Buffer::UnBind() const
	{
		GLCall(glBindBuffer(GetGLBufferTarget(), NULL));
	}

	GLenum Buffer::GetGLUsage() const
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
			return GL_INVALID_ENUM;
		}
	}

	void Buffer::Dispose()
	{
		if (bufferID != NULL)
		{
			GLCall(glDeleteBuffers(1, &bufferID));
			bufferID = NULL;
		}
	}

	// ------------------------------------------------------------------------------------------------
	// --- VertexBuffer:
	// ------------------------------------------------------------------------------------------------

	VertexBuffer::VertexBuffer(BufferUsage usage) : Buffer(usage)
	{
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	GLenum VertexBuffer::GetGLBufferTarget() const
	{
		return GL_ARRAY_BUFFER;
	}

	// ------------------------------------------------------------------------------------------------
	// --- IndexBuffer:
	// ------------------------------------------------------------------------------------------------

	IndexBuffer::IndexBuffer(BufferUsage usage, IndexType type) : Buffer(usage), indexType(type)
	{
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	GLenum IndexBuffer::GetGLIndexType()
	{
		switch (indexType)
		{
		case IndexType::UnsignedByte:
			return GL_UNSIGNED_BYTE;
		case IndexType::UnsignedShort:
			return GL_UNSIGNED_SHORT;
		case IndexType::UnsignedInt:
			return GL_UNSIGNED_INT;
		default:
			assert(false);
			return GL_INVALID_ENUM;
		}
	}

	GLenum IndexBuffer::GetGLBufferTarget() const
	{
		return GL_ELEMENT_ARRAY_BUFFER;
	}

	// ------------------------------------------------------------------------------------------------
	// --- BufferElement:
	// ------------------------------------------------------------------------------------------------

	BufferElement::BufferElement(ShaderDataType type, const char* name, bool normalized) : Name(name), Type(type), Size(0), Offset(0), Normalized(normalized)
	{
	}

	int BufferElement::GetElementCount() const
	{
		switch (Type)
		{
		case ShaderDataType::Byte:
		case ShaderDataType::SByte:
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
		case ShaderDataType::vec4_Byte:
			return 4;
		default:
			assert(false);
			return GL_INVALID_ENUM;
		}
	}

	GLenum BufferElement::GetDataType() const
	{
		switch (Type)
		{
		case ShaderDataType::Byte:
		case ShaderDataType::vec4_Byte:
			return GL_UNSIGNED_BYTE;
		case ShaderDataType::SByte:
			return GL_BYTE;
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
			return GL_INVALID_ENUM;
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
		case ShaderDataType::Byte:
			return sizeof(unsigned char);
		case ShaderDataType::SByte:
			return sizeof(char);
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
		case ShaderDataType::vec4_Byte:
			return sizeof(unsigned char) * 4;
		default:
			assert(false);
			return 0;
		}
	}

	// ------------------------------------------------------------------------------------------------
}

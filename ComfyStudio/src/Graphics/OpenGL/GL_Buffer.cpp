#include "GL_Buffer.h"
#include <assert.h>

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- Buffer:
	// ------------------------------------------------------------------------------------------------

	GL_Buffer::GL_Buffer(BufferUsage usage) : bufferUsage(usage)
	{
	}

	GL_Buffer::~GL_Buffer()
	{
		Dispose();
	}

	void GL_Buffer::InitializeID()
	{
		GLCall(glGenBuffers(1, &bufferID));
	}

	void GL_Buffer::Upload(size_t dataSize, void* data)
	{
		GLCall(glBufferData(GetGLBufferTarget(), dataSize, data, GetGLUsage()));
	}

	void GL_Buffer::UploadSubData(size_t dataSize, size_t* offset, void* data)
	{
		GLCall(glBufferSubData(GetGLBufferTarget(), (GLintptr)offset, dataSize, data));
	}

	void GL_Buffer::Bind() const
	{
		GLCall(glBindBuffer(GetGLBufferTarget(), bufferID));
	}

	void GL_Buffer::UnBind() const
	{
		GLCall(glBindBuffer(GetGLBufferTarget(), NULL));
	}

	void GL_Buffer::SetObjectLabel(const char* label)
	{
		GLCall(glObjectLabel(GL_BUFFER, bufferID, -1, label));
	}

	GLenum GL_Buffer::GetGLUsage() const
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

	void GL_Buffer::Dispose()
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

	GL_VertexBuffer::GL_VertexBuffer(BufferUsage usage) : GL_Buffer(usage)
	{
	}

	GL_VertexBuffer::~GL_VertexBuffer()
	{
	}

	GLenum GL_VertexBuffer::GetGLBufferTarget() const
	{
		return GL_ARRAY_BUFFER;
	}

	// ------------------------------------------------------------------------------------------------
	// --- IndexBuffer:
	// ------------------------------------------------------------------------------------------------

	GL_IndexBuffer::GL_IndexBuffer(BufferUsage usage, IndexType type) : GL_Buffer(usage), indexType(type)
	{
	}

	GL_IndexBuffer::~GL_IndexBuffer()
	{
	}

	GLenum GL_IndexBuffer::GetGLIndexType()
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

	GLenum GL_IndexBuffer::GetGLBufferTarget() const
	{
		return GL_ELEMENT_ARRAY_BUFFER;
	}

	// ------------------------------------------------------------------------------------------------
	// --- BufferElement:
	// ------------------------------------------------------------------------------------------------

	BufferElement::BufferElement(ShaderDataType type, const char* name, bool normalized) : BufferElement(type, name, normalized, nullptr)
	{
	}

	BufferElement::BufferElement(ShaderDataType type, const char* name, bool normalized, const GL_VertexBuffer* buffer) : Name(name), Type(type), Size(0), Offset(0), Normalized(normalized), Buffer(buffer)
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

	BufferLayout::BufferLayout(std::initializer_list<BufferElement> elements) : stride(0), elements(elements)
	{
		UpdateElements();
	}

	void BufferLayout::UpdateElements()
	{
		for (BufferElement& element : elements)
		{
			element.Size = GetElementSize(element.Type);
			element.Offset = element.GetBuffer() != nullptr ? 0 : stride;

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

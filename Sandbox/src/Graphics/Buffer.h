#pragma once
#include "Types.h"
#include "Graphics.h"
#include "GraphicsInterface.h"
#include <vector>

// ------------------------------------------------------------------------------------------------
// --- Buffer:
// ------------------------------------------------------------------------------------------------

typedef GLuint BufferID_t;

enum class BufferUsage
{
	StreamDraw,
	StreamRead,
	StreamCopy,
	StaticDraw,
	StaticRead,
	StaticCopy,
	DynamicDraw,
	DynamicRead,
	DynamicCopy,
};

class Buffer : public IGraphicsObject
{
public:
	Buffer(BufferUsage usage);
	virtual ~Buffer();
	Buffer(const Buffer&) = delete;

	void InitializeID() override;
	void Upload(size_t dataSize, void* data);
	void UploadSubData(size_t dataSize, size_t* offset, void* data);

	void Bind() const override;
	void UnBind() const override;

protected:
	BufferID_t bufferID = NULL;
	BufferUsage bufferUsage;

	GLenum GetGLUsage() const;
	virtual GLenum GetGLBufferTarget() const = 0;

	void Dispose();
};

// ------------------------------------------------------------------------------------------------
// --- VertexBuffer:
// ------------------------------------------------------------------------------------------------

class VertexBuffer : public Buffer
{
public:
	VertexBuffer(BufferUsage usage);
	~VertexBuffer();
	VertexBuffer(const VertexBuffer&) = delete;

protected:
	GLenum GetGLBufferTarget() const override;
};

// ------------------------------------------------------------------------------------------------
// --- IndexBuffer:
// ------------------------------------------------------------------------------------------------

enum class IndexType
{
	UnsignedByte,
	UnsignedShort,
	UnsignedInt,
};

class IndexBuffer : public Buffer
{
public:
	IndexBuffer(BufferUsage usage, IndexType type);
	~IndexBuffer();
	IndexBuffer(const IndexBuffer&) = delete;

	GLenum GetGLIndexType();

protected:
	IndexType indexType;

	GLenum GetGLBufferTarget() const override;
};

// ------------------------------------------------------------------------------------------------
// --- ShaderDataType:
// ------------------------------------------------------------------------------------------------

enum class ShaderDataType : uint16_t
{
	Int, 
	Float, 
	Bool, 
	Vec2, 
	Vec3, 
	Vec4, 
	Mat3, 
	Mat4,
};

// ------------------------------------------------------------------------------------------------
// --- BufferElement:
// ------------------------------------------------------------------------------------------------

struct BufferElement
{
	const char* Name;
	ShaderDataType Type;
	uint16_t Size;
	uint16_t Offset;
	bool Normalized;

	BufferElement(ShaderDataType type, const char* name);
	
	int GetElementCount() const;
	GLenum GetDataType() const;
	bool GetIsNormalized() const;
	inline void* GetOffset() const { return (void*)Offset; };
};

// ------------------------------------------------------------------------------------------------
// --- BufferLayout:
// ------------------------------------------------------------------------------------------------

class BufferLayout
{
public:
	BufferLayout(std::initializer_list<BufferElement> elements);

	inline uint16_t GetStride() const { return stride; };
	inline const std::vector<BufferElement>& GetElemenets() const { return elements; };

private:
	uint16_t stride;
	std::vector<BufferElement> elements;

	void UpdateElements();
	static uint16_t GetElementSize(ShaderDataType type);
};

// ------------------------------------------------------------------------------------------------

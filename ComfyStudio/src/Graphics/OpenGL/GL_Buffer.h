#pragma once
#include "Types.h"
#include "OpenGL.h"
#include "Graphics/GraphicsTypes.h"
#include "Core/CoreTypes.h"

namespace Graphics
{
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

	class GL_Buffer /*: public IGraphicsObject*/
	{
	public:
		GL_Buffer(BufferUsage usage);
		GL_Buffer(const GL_Buffer&) = delete;
		GL_Buffer& operator= (const GL_Buffer&) = delete;
		virtual ~GL_Buffer();

		void InitializeID() /*override*/;
		void Upload(size_t dataSize, void* data);
		void UploadSubData(size_t dataSize, size_t* offset, void* data);

		void Bind() const /*override*/;
		void UnBind() const /*override*/;
		void SetObjectLabel(const char* label) /*override*/;

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

	class GL_VertexBuffer : public GL_Buffer
	{
	public:
		GL_VertexBuffer(BufferUsage usage);
		GL_VertexBuffer(const GL_VertexBuffer&) = delete;
		GL_VertexBuffer& operator= (const GL_VertexBuffer&) = delete;
		virtual ~GL_VertexBuffer();

	protected:
		GLenum GetGLBufferTarget() const override;
	};

	// ------------------------------------------------------------------------------------------------
	// --- IndexBuffer:
	// ------------------------------------------------------------------------------------------------

	class GL_IndexBuffer : public GL_Buffer
	{
	public:
		GL_IndexBuffer(BufferUsage usage, IndexType type);
		GL_IndexBuffer(const GL_IndexBuffer&) = delete;
		GL_IndexBuffer& operator= (const GL_IndexBuffer&) = delete;
		virtual ~GL_IndexBuffer();

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
		Byte,
		SByte,
		Int,
		Float,
		Bool,
		Vec2,
		Vec3,
		Vec4,
		Mat3,
		Mat4,
		vec4_Byte,
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
		const GL_VertexBuffer* Buffer;

		BufferElement(ShaderDataType type, const char* name, bool normalized = false);
		BufferElement(ShaderDataType type, const char* name, bool normalized, const GL_VertexBuffer* buffer);

		int GetElementCount() const;
		GLenum GetDataType() const;
		bool GetIsNormalized() const;
		inline void* GetOffset() const { return reinterpret_cast<void*>(Offset); };
		inline const GL_VertexBuffer* GetBuffer() const { return Buffer; };
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
}

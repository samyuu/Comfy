#pragma once
#include "Texture/Texture2D.h"
#include "Graphics.h"
#include "GraphicsInterface.h"

namespace Graphics
{
	// ------------------------------------------------------------------------------------------------
	// --- Renderbuffer:
	// ------------------------------------------------------------------------------------------------

	typedef GLuint RenderbufferID_t;
	typedef GLenum RenderTarget_t;
	typedef GLenum InternalFormat_t;

	class Renderbuffer : public IGraphicsObject
	{
	public:
		Renderbuffer();
		Renderbuffer(const Renderbuffer&) = delete;
		~Renderbuffer();

		void InitializeID() override;
		void Bind() const override;
		void UnBind() const override;
		void RenderbufferStorage(ivec2 size, InternalFormat_t internalFormat);
		void SetObjectLabel(const char* label) override;

		inline RenderbufferID_t GetRenderbufferID() const { return renderbufferID; };
		inline RenderTarget_t GetRenderTarget() const { return GL_RENDERBUFFER; };

	protected:
		RenderbufferID_t renderbufferID = NULL;

		void Dispose();
	};

	// ------------------------------------------------------------------------------------------------
	// --- Framebuffer:
	// ------------------------------------------------------------------------------------------------

	typedef GLuint FramebufferID_t;
	typedef GLenum BufferTarget_t;
	typedef GLenum FramebufferStatus_t;
	typedef GLenum Attachment_t;

	class Framebuffer : public IGraphicsObject
	{
	public:
		Framebuffer();
		Framebuffer(const Framebuffer&) = delete;
		~Framebuffer();

		void InitializeID() override;

		void Bind() const override;
		void UnBind() const override;

		FramebufferStatus_t CheckStatus();
		void AttachTexture(Texture2D& texture, Attachment_t attachment);
		void AttachRenderbuffer(Renderbuffer& renderbuffer, Attachment_t attachment);
		void SetObjectLabel(const char* label) override;

	protected:
		FramebufferID_t framebufferID = NULL;

		inline BufferTarget_t GetBufferTarget() const { return GL_FRAMEBUFFER; };
		void Dispose();
	};

	// ------------------------------------------------------------------------------------------------
	// --- RenderTarget:
	// ------------------------------------------------------------------------------------------------

	class RenderTarget
	{
	public:
		RenderTarget();
		RenderTarget(const RenderTarget&) = delete;
		~RenderTarget();

		void Initialize(ivec2 size);
		void Bind();
		void UnBind();

		void Resize(ivec2 size);

		inline Texture2D& GetTexture() { return colorTexture; };
		inline void* GetVoidTexture() { return GetTexture().GetVoidTexture(); };
		inline float GetWidth() const { return dimensions.x; };
		inline float GetHeight() const { return dimensions.y; };
		inline const vec2& GetSize() const { return dimensions; };

		inline Framebuffer& GetFramebuffer() { return framebuffer; };
		inline Texture2D& GetColorTexture() { return colorTexture; };
		inline Renderbuffer& GetDepthBuffer() { return depthRenderbuffer; };

	protected:
		vec2 dimensions;

		Framebuffer framebuffer;
		Texture2D colorTexture;
		Renderbuffer depthRenderbuffer;

		void Dispose();
	};

	// ------------------------------------------------------------------------------------------------
}

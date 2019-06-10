#pragma once
#include "../pch.h"
#include "Texture.h"

// ------------------------------------------------------------------------------------------------
// --- Renderbuffer:
// ------------------------------------------------------------------------------------------------

typedef GLuint RenderbufferID_t;
typedef GLenum RenderTarget_t;
typedef GLenum InternalFormat_t;

class Renderbuffer
{
public:
	Renderbuffer();
	~Renderbuffer();
	Renderbuffer(const Renderbuffer&) = delete;

	void Initialize();
	void Bind();
	void UnBind();
	void RenderbufferStorage(int width, int height, InternalFormat_t internalFormat);

	inline RenderbufferID_t GetRenderbufferID() { return renderbufferID; };
	inline RenderTarget_t GetRenderTarget() { return GL_RENDERBUFFER; };

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

class Framebuffer
{
public:
	Framebuffer();
	~Framebuffer();
	Framebuffer(const Framebuffer&) = delete;

	void Initialize();

	void Bind();
	void UnBind();

	FramebufferStatus_t CheckStatus();
	void AttachTexture(Texture& texture, Attachment_t attachment);
	void AttachRenderbuffer(Renderbuffer& renderbuffer, Attachment_t attachment);

protected:
	FramebufferID_t framebufferID = NULL;

	inline BufferTarget_t GetBufferTarget() { return GL_FRAMEBUFFER; };
	void Dispose();
};

// ------------------------------------------------------------------------------------------------
// --- RenderTarget:
// ------------------------------------------------------------------------------------------------

class RenderTarget
{
public:
	RenderTarget();
	~RenderTarget();
	RenderTarget(const RenderTarget&) = delete;

	void Initialize(int width, int height);
	void Bind();
	void UnBind();

	void Resize(int width, int height);

	inline Texture& GetTexture() { return colorTexture; };
	inline void* GetVoidTexture() { return GetTexture().GetVoidTexture(); };
	inline int GetWidth() { return width; };
	inline int GetHeight() { return height; };

protected:
	int width, height;

	Framebuffer framebuffer;
	Texture colorTexture;
	Renderbuffer depthRenderbuffer;

	void Dispose();
};

// ------------------------------------------------------------------------------------------------

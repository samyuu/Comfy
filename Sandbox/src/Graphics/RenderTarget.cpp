#include "RenderTarget.h"
#include <assert.h>

// ------------------------------------------------------------------------------------------------
// --- Renderbuffer:
// ------------------------------------------------------------------------------------------------

Renderbuffer::Renderbuffer()
{
}

Renderbuffer::~Renderbuffer()
{
	Dispose();
}

void Renderbuffer::InitializeID()
{
	GLCall(glGenRenderbuffers(1, &renderbufferID));
}

void Renderbuffer::Bind() const
{
	GLCall(glBindRenderbuffer(GetRenderTarget(), renderbufferID));
}

void Renderbuffer::UnBind() const
{
	GLCall(glBindRenderbuffer(GetRenderTarget(), NULL));
}

void Renderbuffer::RenderbufferStorage(int width, int height, InternalFormat_t internalFormat)
{
	GLCall(glRenderbufferStorage(GetRenderTarget(), internalFormat, width, height));
}

void Renderbuffer::Dispose()
{
	if (renderbufferID != NULL)
	{
		GLCall(glDeleteRenderbuffers(1, &renderbufferID));
		renderbufferID = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
// --- Framebuffer:
// ------------------------------------------------------------------------------------------------

Framebuffer::Framebuffer()
{
}

Framebuffer::~Framebuffer()
{
	Dispose();
}

void Framebuffer::Initialize()
{
	GLCall(glGenFramebuffers(1, &framebufferID));
}

void Framebuffer::Bind()
{
	GLCall(glBindFramebuffer(GetBufferTarget(), framebufferID));
}

void Framebuffer::UnBind()
{
	GLCall(glBindFramebuffer(GetBufferTarget(), NULL));
}

FramebufferStatus_t Framebuffer::CheckStatus()
{
	GLenum framebufferStatus;
	GLCall(framebufferStatus = glCheckFramebufferStatus(GetBufferTarget()));
	return framebufferStatus;
}

void Framebuffer::AttachTexture(Texture2D& texture, Attachment_t attachment)
{
	GLCall(glFramebufferTexture2D(GetBufferTarget(), attachment, texture.GetTextureTarget(), texture.GetTextureID(), 0));
}

void Framebuffer::AttachRenderbuffer(Renderbuffer& renderbuffer, Attachment_t attachment)
{
	GLCall(glFramebufferRenderbuffer(GetBufferTarget(), attachment, renderbuffer.GetRenderTarget(), renderbuffer.GetRenderbufferID()));
}

void Framebuffer::Dispose()
{
	if (framebufferID != NULL)
	{
		GLCall(glDeleteFramebuffers(1, &framebufferID));
		framebufferID = NULL;
	}
}

// ------------------------------------------------------------------------------------------------
// --- RenderTarget:
// ------------------------------------------------------------------------------------------------

RenderTarget::RenderTarget()
{
}

RenderTarget::~RenderTarget()
{
	Dispose();
}

void RenderTarget::Initialize(int width, int height)
{
	framebuffer.Initialize();
	framebuffer.Bind();

	colorTexture.InitializeID();
	depthRenderbuffer.InitializeID();
	Resize(width, height);

	framebuffer.AttachRenderbuffer(depthRenderbuffer, GL_DEPTH_STENCIL_ATTACHMENT);
	assert(framebuffer.CheckStatus() == GL_FRAMEBUFFER_COMPLETE);
	framebuffer.UnBind();
}

void RenderTarget::Bind()
{
	framebuffer.Bind();
}

void RenderTarget::UnBind()
{
	framebuffer.UnBind();
}

void RenderTarget::Resize(int width, int height)
{
	dimensions = vec2(width, height);

	colorTexture.Bind();
	colorTexture.UploadEmpty(width, height);
	colorTexture.UnBind();

	framebuffer.Bind();
	framebuffer.AttachTexture(colorTexture, GL_COLOR_ATTACHMENT0);

	depthRenderbuffer.Bind();
	depthRenderbuffer.RenderbufferStorage(width, height, GL_DEPTH24_STENCIL8);
	depthRenderbuffer.UnBind();
}

void RenderTarget::Dispose()
{

}

// ------------------------------------------------------------------------------------------------

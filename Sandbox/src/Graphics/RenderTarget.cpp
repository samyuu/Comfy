#include "../pch.h"
#include "RenderTarget.h"

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

void Renderbuffer::Initialize()
{
	glGenRenderbuffers(1, &renderbufferID);
}

void Renderbuffer::Bind()
{
	glBindRenderbuffer(GetRenderTarget(), renderbufferID);
}

void Renderbuffer::UnBind()
{
	glBindRenderbuffer(GetRenderTarget(), NULL);
}

void Renderbuffer::RenderbufferStorage(int width, int height, InternalFormat_t internalFormat)
{
	glRenderbufferStorage(GetRenderTarget(), internalFormat, width, height);
}

void Renderbuffer::Dispose()
{
	if (renderbufferID != NULL)
		glDeleteRenderbuffers(1, &renderbufferID);
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
	glGenFramebuffers(1, &framebufferID);

}

void Framebuffer::Bind()
{
	glBindFramebuffer(GetBufferTarget(), framebufferID);
}

void Framebuffer::UnBind()
{
	glBindFramebuffer(GetBufferTarget(), NULL);
}

FramebufferStatus_t Framebuffer::CheckStatus()
{
	return glCheckFramebufferStatus(GetBufferTarget());
}

void Framebuffer::AttachTexture(Texture& texture, Attachment_t attachment)
{
	glFramebufferTexture2D(GetBufferTarget(), attachment, texture.GetTextureTarget(), texture.GetTextureID(), 0);
}

void Framebuffer::AttachRenderbuffer(Renderbuffer& renderbuffer, Attachment_t attachment)
{
	glFramebufferRenderbuffer(GetBufferTarget(), attachment, renderbuffer.GetRenderTarget(), renderbuffer.GetRenderbufferID());
}

void Framebuffer::Dispose()
{
	if (framebufferID != NULL)
		glDeleteFramebuffers(1, &framebufferID);
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

	colorTexture.Initialize();
	depthRenderbuffer.Initialize();
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
	this->width = width;
	this->height = height;

	colorTexture.Bind();
	colorTexture.GenerateEmpty(width, height);
	colorTexture.UnBind();
	framebuffer.AttachTexture(colorTexture, GL_COLOR_ATTACHMENT0);

	depthRenderbuffer.Bind();
	depthRenderbuffer.RenderbufferStorage(width, height, GL_DEPTH24_STENCIL8);
	depthRenderbuffer.UnBind();
}

void RenderTarget::Dispose()
{

}

// ------------------------------------------------------------------------------------------------

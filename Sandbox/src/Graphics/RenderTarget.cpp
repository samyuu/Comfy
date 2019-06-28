#include "RenderTarget.h"
#include "ErrorChecking.h"
#include <glad/glad.h>
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
	glGenRenderbuffers(1, &renderbufferID);
	CHECK_GL_ERROR("glGenRenderbuffers()");
}

void Renderbuffer::Bind() const
{
	glBindRenderbuffer(GetRenderTarget(), renderbufferID);
}

void Renderbuffer::UnBind() const
{
	glBindRenderbuffer(GetRenderTarget(), NULL);
}

void Renderbuffer::RenderbufferStorage(int width, int height, InternalFormat_t internalFormat)
{
	glRenderbufferStorage(GetRenderTarget(), internalFormat, width, height);
	CHECK_GL_ERROR("glRenderbufferStorage()");
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
	CHECK_GL_ERROR("glGenFramebuffers()");
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

void Framebuffer::AttachTexture(Texture2D& texture, Attachment_t attachment)
{
	glFramebufferTexture2D(GetBufferTarget(), attachment, texture.GetTextureTarget(), texture.GetTextureID(), 0);
	CHECK_GL_ERROR("glFramebufferTexture2D()");
}

void Framebuffer::AttachRenderbuffer(Renderbuffer& renderbuffer, Attachment_t attachment)
{
	glFramebufferRenderbuffer(GetBufferTarget(), attachment, renderbuffer.GetRenderTarget(), renderbuffer.GetRenderbufferID());
	CHECK_GL_ERROR("glFramebufferRenderbuffer()");
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
	this->width = width;
	this->height = height;

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

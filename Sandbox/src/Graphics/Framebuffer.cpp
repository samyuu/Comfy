#include "Framebuffer.h"

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
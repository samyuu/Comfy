#include "Renderbuffer.h"

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
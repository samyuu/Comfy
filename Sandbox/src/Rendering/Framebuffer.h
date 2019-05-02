#pragma once
#include "../pch.h"
#include "Texture.h"
#include "Renderbuffer.h"

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


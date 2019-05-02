#pragma once
#include "../pch.h"

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


#include "RenderTarget.h"
#include <assert.h>

namespace Graphics
{
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

	void Renderbuffer::RenderbufferStorage(ivec2 size, InternalFormat_t internalFormat)
	{
		GLCall(glRenderbufferStorage(GetRenderTarget(), internalFormat, size.x, size.y));
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

	void RenderTarget::Initialize(ivec2 size)
	{
		assert(size.x > 0 && size.y > 0);

		framebuffer.Initialize();
		framebuffer.Bind();

		colorTexture.InitializeID();
		depthRenderbuffer.InitializeID();
		Resize(size);

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

	void RenderTarget::Resize(ivec2 size)
	{
		constexpr ivec2 minimumSize = ivec2(1.0, 1.0);
		dimensions = glm::max(minimumSize, size);

		colorTexture.Bind();
		colorTexture.UploadEmpty(size);
		colorTexture.UnBind();

		framebuffer.Bind();
		framebuffer.AttachTexture(colorTexture, GL_COLOR_ATTACHMENT0);

		depthRenderbuffer.Bind();
		depthRenderbuffer.RenderbufferStorage(size, GL_DEPTH24_STENCIL8);
		depthRenderbuffer.UnBind();
	}

	void RenderTarget::Dispose()
	{

	}

	// ------------------------------------------------------------------------------------------------
}

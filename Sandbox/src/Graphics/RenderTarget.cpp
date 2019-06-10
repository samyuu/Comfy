#include "RenderTarget.h"

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
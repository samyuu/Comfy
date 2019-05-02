#pragma once
#include "../pch.h"
#include "Renderbuffer.h"
#include "Framebuffer.h"

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


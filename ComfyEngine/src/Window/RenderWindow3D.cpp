#include "RenderWindow.h"

namespace Comfy
{
	// TODO:
	struct RenderWindow3D::Impl
	{
	};

	RenderWindow3D::RenderWindow3D() : impl(std::make_unique<Impl>())
	{
	}

	RenderWindow3D::~RenderWindow3D()
	{
	}

	ImTextureID RenderWindow3D::GetTextureID() const
	{
		return nullptr;
	}

	ImGuiWindowFlags RenderWindow3D::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void RenderWindow3D::OnFirstFrame()
	{
	}

	void RenderWindow3D::PreRenderTextureGui()
	{
	}

	void RenderWindow3D::PostRenderTextureGui()
	{
	}

	void RenderWindow3D::OnResize(ivec2 newSize)
	{
	}

	void RenderWindow3D::OnRender()
	{
	}
}

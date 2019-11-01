#pragma once
#include "Graphics/OpenGL/GL_Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "Core/TimeSpan.h"

namespace App
{
	class TaskInterface
	{
	public:
		virtual bool Initialize() = 0;
		virtual bool Update() = 0;
		virtual bool Render(Graphics::GL_Renderer2D* renderer, Graphics::AetRenderer* aetRenderer) = 0;
		virtual bool PreDrawGui() = 0;
		virtual bool PostDrawGui() = 0;
	};

	class Task : public TaskInterface
	{
	public:
		virtual bool PreDrawGui() override { return true; };
		virtual bool PostDrawGui() override { return true; };

	protected:
		TimeSpan elapsedTime;
	};
}
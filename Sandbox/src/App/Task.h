#pragma once
#include "Graphics/Auth2D/Renderer2D.h"
#include "TimeSpan.h"

namespace App
{
	class TaskInterface
	{
	public:
		virtual bool Initialize() = 0;
		virtual bool Update() = 0;
		virtual bool Render(Auth2D::Renderer2D& renderer) = 0;
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
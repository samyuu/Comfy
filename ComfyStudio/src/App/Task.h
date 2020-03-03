#pragma once
#include "Graphics/Auth2D/AetRenderer.h"
#include "Core/TimeSpan.h"

namespace Comfy::App
{
	class TaskInterface
	{
	public:
		virtual bool Initialize() = 0;
		virtual bool Update() = 0;
		virtual bool Render(Graphics::D3D_Renderer2D* renderer, Graphics::AetRenderer* aetRenderer) = 0;
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

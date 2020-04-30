#pragma once
#include "Graphics/Auth2D/Aet/AetRenderer.h"
#include "Time/TimeSpan.h"

namespace Comfy::App
{
	class TaskInterface
	{
	public:
		virtual bool Initialize() = 0;
		virtual bool Update() = 0;
		virtual bool Render(Graphics::GPU_Renderer2D* renderer, Graphics::Aet::AetRenderer* aetRenderer) = 0;
		virtual bool PreDrawGui() = 0;
		virtual bool PostDrawGui() = 0;
	};

	class Task : public TaskInterface
	{
	public:
		bool PreDrawGui() override { return true; }
		bool PostDrawGui() override { return true; }

	protected:
		TimeSpan elapsedTime;
	};
}

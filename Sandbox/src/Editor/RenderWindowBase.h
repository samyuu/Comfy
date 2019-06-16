#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/imgui_extensions.h"
#include "../Graphics/RenderTarget.h"

namespace Editor
{
	constexpr int RENDER_TARGET_DEFAULT_WIDTH = 1;
	constexpr int RENDER_TARGET_DEFAULT_HEIGHT = 1;

	class RenderWindowBase
	{
	public:
		void Initialize();
		void DrawGui();

		inline bool GetKeepAspectRatio() { return keepAspectRatio; };
		inline void SetKeepAspectRatio(bool value) { keepAspectRatio = value; };

		inline float GetTargetAspectRatio() { return targetAspectRatio; };
		inline void SetTargetAspectRatio(float value) { targetAspectRatio = value; };

	protected:
		ImRect renderRegion, lastRenderRegion;
		RenderTarget renderTarget;

		virtual void OnUpdateInput() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(int width, int height);

		inline bool GetWasResized() { return wasResized; };

	private:
		bool wasResized = false;

		bool keepAspectRatio = false;
		float targetAspectRatio = 16.0f / 9.0f;
	};
}
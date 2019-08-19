#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include "ImGui/Gui.h"
#include "Graphics/RenderTarget.h"
#include "Graphics/RenderCommand.h"

namespace Editor
{
	constexpr int RENDER_TARGET_DEFAULT_WIDTH = 1;
	constexpr int RENDER_TARGET_DEFAULT_HEIGHT = 1;

	class RenderWindowBase
	{
	public:
		RenderWindowBase() {};
		virtual ~RenderWindowBase() {};

		void Initialize();
		void DrawGui();

		vec2 GetRelativeMouse() const;

		inline bool GetKeepAspectRatio() { return keepAspectRatio; };
		inline void SetKeepAspectRatio(bool value) { keepAspectRatio = value; };

		inline float GetTargetAspectRatio() { return targetAspectRatio; };
		inline void SetTargetAspectRatio(float value) { targetAspectRatio = value; };

		static inline void PushWindowPadding() { Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(2, 2)); };
		static inline void PopWindowPadding() { Gui::PopStyleVar(); };

	protected:
		Graphics::RenderTarget renderTarget;

		virtual ImGuiWindowFlags GetChildWinodwFlags() const { return ImGuiWindowFlags_None; };

		virtual void OnInitialize() {};
		virtual void OnDrawGui() {};
		virtual void PostDrawGui() {};
		virtual void OnUpdateInput() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(int width, int height);

		inline bool GetWasResized() { return wasResized; };
		const ImRect& GetRenderRegion() const;

	private:
		ImRect renderRegion, lastRenderRegion;
		bool wasResized = false;
		bool keepAspectRatio = false;
		float targetAspectRatio = 16.0f / 9.0f;
	};
}
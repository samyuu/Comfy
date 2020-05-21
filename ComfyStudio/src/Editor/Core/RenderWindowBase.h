#pragma once
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class RenderWindowBase : NonCopyable
	{
	public:
		RenderWindowBase() = default;
		virtual ~RenderWindowBase() = default;

	public:
		void Initialize();
		void DrawGui();

		vec2 GetRelativeMouse() const;

		inline bool GetKeepAspectRatio() const { return keepAspectRatio; }
		inline void SetKeepAspectRatio(bool value) { keepAspectRatio = value; }

		inline float GetTargetAspectRatio() const { return targetAspectRatio; }
		inline void SetTargetAspectRatio(float value) { targetAspectRatio = value; }

		inline bool GetWasResized() const { return wasResized; }
		inline const ImRect& GetRenderRegion() const { return renderRegion; }

	public:
		static inline void PushWindowPadding() { Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(2.0f, 2.0f)); }
		static inline void PopWindowPadding() { Gui::PopStyleVar(); }

	protected:
		std::unique_ptr<Graphics::GPU_RenderTarget> owningRenderTarget = nullptr;

		virtual ImGuiWindowFlags GetChildWinodwFlags() const { return ImGuiWindowFlags_None; }

		virtual void OnInitialize() {}
		virtual void OnDrawGui() {}
		virtual void PostDrawGui() {}
		virtual void OnUpdateInput() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(ivec2 size);

		virtual bool GetShouldCreateDepthRenderTarget() const { return false; }
		virtual Graphics::GPU_RenderTarget* GetExternalRenderTarget() { return nullptr; }

	private:
		ImRect renderRegion, lastRenderRegion;
		bool resizeOwningRenderTarget = true;
		bool wasResized = false;
		bool needsResizing = true;
		bool keepAspectRatio = false;
		float targetAspectRatio = 16.0f / 9.0f;
	};
}

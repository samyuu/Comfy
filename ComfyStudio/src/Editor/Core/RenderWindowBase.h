#pragma once
#include "ImGui/Gui.h"
#include "Graphics/Direct3D/D3D_RenderTarget.h"

namespace Editor
{
	class RenderWindowBase
	{
	public:
		// NOTE: Since the render target is stretched to the correct asspect ratio in the end it could easily be scaled down for weaker hardware
		static constexpr ivec2 RenderTargetDefaultSize = ivec2(4, 4);

	public:
		RenderWindowBase() = default;
		RenderWindowBase(const RenderWindowBase&) = delete;
		virtual ~RenderWindowBase() = default;

	public:
		void Initialize();
		void DrawGui();

		vec2 GetRelativeMouse() const;

		inline bool GetKeepAspectRatio() const { return keepAspectRatio; };
		inline void SetKeepAspectRatio(bool value) { keepAspectRatio = value; };

		inline float GetTargetAspectRatio() const { return targetAspectRatio; };
		inline void SetTargetAspectRatio(float value) { targetAspectRatio = value; };

	public:
		static inline void PushWindowPadding() { Gui::PushStyleVar(ImGuiStyleVar_WindowPadding, vec2(2.0f, 2.0f)); };
		static inline void PopWindowPadding() { Gui::PopStyleVar(); };

	protected:
		Graphics::D3D_RenderTarget renderTarget = { RenderTargetDefaultSize };

		virtual ImGuiWindowFlags GetChildWinodwFlags() const { return ImGuiWindowFlags_None; };

		virtual void OnInitialize() {};
		virtual void OnDrawGui() {};
		virtual void PostDrawGui() {};
		virtual void OnUpdateInput() = 0;
		virtual void OnUpdate() = 0;
		virtual void OnRender() = 0;
		virtual void OnResize(ivec2 size);

		inline bool GetWasResized() const { return wasResized; };
		const ImRect& GetRenderRegion() const;

	private:
		ImRect renderRegion, lastRenderRegion;
		bool wasResized = false;
		bool needsResizing = true;
		bool keepAspectRatio = false;
		float targetAspectRatio = 16.0f / 9.0f;
	};
}
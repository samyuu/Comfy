#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Render/Render.h"
#include "ImGui/Extensions/CheckerboardTexture.h"
#include <functional>

namespace Comfy
{
	class RenderWindow : NonCopyable
	{
	public:
		RenderWindow() = default;
		virtual ~RenderWindow() = default;

	public:
		void BeginEndGui(const char* windowName, bool* isOpen = nullptr, ImGuiWindowFlags flags = ImGuiWindowFlags_None);

		// NOTE: Covering the entire main viewport without pre/post gui widgets.
		//		 It is recommended to use a different window name ID than the non fullscreen version to avoid changing its original layout
		//		 and making sure no other windows are being drawn after this as they won't be visible
		void BeginEndGuiFullScreen(const char* windowName);

	public:
		vec2 GetRelativeMouse() const;

		bool GetKeepAspectRatio() const;
		void SetKeepAspectRatio(bool value);

		float GetTargetAspectRatio() const;
		void SetTargetAspectRatio(float value);

		// NOTE: Render checkerboard as a fixed aspect ratio background
		bool GetWindowBackgroundCheckerboardEnabled() const;
		void SetWindowBackgroundCheckerboardEnabled(bool value);

		// NOTE: Render checkerboard behind the render target texture
		bool GetRenderBackgroundCheckerboardEnabled() const;
		void SetRenderBackgroundCheckerboardEnabled(bool value);

		bool GetWasResized() const;
		ImRect GetRenderRegion() const;

	public:
		virtual ImTextureID GetTextureID() const = 0;

	protected:
		virtual ImGuiWindowFlags GetRenderTextureChildWindowFlags() const = 0;
		virtual void PreBeginWindow() {};
		virtual void PreRenderTextureGui() = 0;
		virtual void PostRenderTextureGui() = 0;
		virtual void OnResize(ivec2 newSize) = 0;
		virtual void OnRender() = 0;

	private:
		void RenderTextureGui();
		void UpdateRenderRegion();

	private:
		ImRect renderRegion = {}, lastRenderRegion = {};
		ImRect fullRenderRegion = {};

		bool wasResized = false;
		bool needsResizing = true;

		bool keepAspectRatio = false;
		float targetAspectRatio = (16.0f / 9.0f);

		bool windowBackgroundCheckerboardEnabled = false;
		bool renderBackgroundCheckerboardEnabled = false;
		std::optional<Gui::CheckerboardTexture> windowBackgroundCheckerboard, renderBackgroundCheckerboard;
	};
}

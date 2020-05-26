#pragma once
#include "Types.h"
#include "ImGui/Gui.h"
#include "Render/Render.h"
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

		bool GetWasResized() const;
		ImRect GetRenderRegion() const;

	public:
		virtual ImTextureID GetTextureID() const = 0;

	protected:
		virtual ImGuiWindowFlags GetRenderTextureChildWindowFlags() const = 0;
		virtual void PreBeginWindow() {};
		virtual void OnFirstFrame() = 0;
		virtual void PreRenderTextureGui() = 0;
		virtual void PostRenderTextureGui() = 0;
		virtual void OnResize(ivec2 newSize) = 0;
		virtual void OnRender() = 0;

	private:
		void CheckUpdateOnFirstFrame();
		void RenderTextureGui();
		void UpdateRenderRegion();
		void AdjustSizeToTargetAspectRatio();

	private:
		bool isFirstFrame = true;

		ImRect renderRegion = {}, lastRenderRegion = {};
		ImRect fullRenderRegion = {};

		bool wasResized = false;
		bool needsResizing = true;

		bool keepAspectRatio = false;
		float targetAspectRatio = (16.0f / 9.0f);
	};
}

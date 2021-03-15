#pragma once
#include "ImGui/Gui.h"

namespace Comfy::Studio
{
	class ComfyStudioApplication;

	class BaseWindow
	{
	public:
		static constexpr ImGuiWindowFlags NoWindowFlags =
			ImGuiWindowFlags_NoTitleBar |
			ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoInputs |
			ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_NoFocusOnAppearing |
			ImGuiWindowFlags_NoBringToFrontOnFocus;

	public:
		BaseWindow(ComfyStudioApplication& parent) : parentApplication(parent) {}
		virtual ~BaseWindow() = default;

	public:
		virtual const char* GetName() const = 0;
		virtual ImGuiWindowFlags GetFlags() const { return ImGuiWindowFlags_None; }
		virtual void Gui() = 0;

	public:
		inline bool& GetIsOpen() { return isOpen; }
		inline void Close() { isOpen = false; }

	protected:
		bool isOpen = true;
		ComfyStudioApplication& parentApplication;
	};
}

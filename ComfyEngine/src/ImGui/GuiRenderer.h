#pragma once
#include "Gui.h"
#include "Window/ApplicationHost.h"

namespace ImGui
{
	class GuiRenderer
	{
	public:
		GuiRenderer(Comfy::ApplicationHost& host);
		~GuiRenderer();

		bool Initialize();
		void BeginFrame();
		void EndFrame();
		void Dispose();

		bool IsAnyViewportFocused() const;

	private:
		bool InitializeCreateContext();
		bool InitializeSetStartupIoState();
		bool InitializeLoadFontData();
		bool InitializeSetStyle();
		bool InitializeBackend();

		const ImWchar* GetFontGlyphRange() const;
		const ImWchar* GetIconGlyphRange() const;

		ImFontConfig GetIconFontConfig() const;

	private:
		Comfy::ApplicationHost& host;
		const std::array<ImWchar, 3> iconFontGlyphRange;
		
	private:
		static constexpr bool preLoadImGuiConfig = false;
		static constexpr bool restoreConfigWindowSize = false;

		static constexpr const char* configFileName = "imgui.ini";
		static constexpr const char* logFileName = "imgui_log.txt";
		static constexpr const char* fontDirectoryName = "font";
		static constexpr const char* fontFileName = "noto_sans_cjk_jp-regular.otf";

		static constexpr float fontSize = 16.0f;
		static constexpr float iconFontSize = fontSize - 2.0f;

		static constexpr float iconMinAdvanceX = 13.0f;
	};
}

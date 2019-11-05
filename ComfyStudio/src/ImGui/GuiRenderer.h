#pragma once
#include "Gui.h"
#include "Core/ApplicationHost.h"

namespace ImGui
{
	class GuiRenderer
	{
	public:
		GuiRenderer(const ApplicationHost& host);
		~GuiRenderer();

		bool Initialize();
		void BeginFrame();
		void EndFrame();
		void Dispose();

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
		const ApplicationHost& host;
		const ImWchar iconFontGlyphRange[3];
		
	private:
		static constexpr const char* configFileName = "imgui.ini";
		static constexpr const char* logFileName = "imgui_log.txt";
		static constexpr const char* fontFarcFileName = "rom/font.farc";
		static constexpr const char* fontFileName = "NotoSansCJKjp-Regular.otf";

		static constexpr float fontSize = 16.0f;
		static constexpr float iconFontSize = fontSize - 2.0f;

		static constexpr float iconMinAdvanceX = 13.0f;
	};
}
#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Gui.h"
#include "Window/ApplicationHost.h"

namespace ImGui
{
	class GuiRenderer
	{
	public:
		// NOTE: Used for setting the initial window size and position
		static constexpr const char* MainDockSpaceID = "MainDockSpace";

	public:
		GuiRenderer(Comfy::ApplicationHost& host);
		~GuiRenderer() = default;

	public:
		bool Initialize();
		void BeginFrame();
		void EndFrame();
		void Dispose();

		bool IsAnyViewportFocused() const;

	private:
		bool CreateImGuiContext();
		bool SetStartupIOState();
		bool LoadFontFiles();
		bool InitializeFonts();
		bool SetComfyStyle();
		bool InitializeBackend();

		const ImWchar* GetTextGlyphRange() const;
		const ImWchar* GetIconGlyphRange() const;

	private:
		Comfy::ApplicationHost& host;

		std::unique_ptr<u8[]> combinedFontFileContent = nullptr;
		size_t textFontFileSize = 0, iconFontFileSize = 0;

#ifdef IMGUI_HACKS_RECORD_MISSING_GLYPHS
		bool buildFullTextGlyphRange = false;
		bool fullFontRangeHasBeenRebuilt = false;
#else
		bool buildFullTextGlyphRange = true;
#endif

	private:
		static constexpr bool preLoadImGuiConfig = false;
		static constexpr bool restoreConfigWindowSize = false;

		static constexpr const char* configFileName = "imgui.ini";
		static constexpr const char* logFileName = "imgui_log.txt";
		static constexpr std::string_view fontDirectoryName = "font";
		static constexpr std::string_view textFontName = "Noto Sans CJK JP";
		static constexpr std::string_view textFontFileName = "noto_sans_cjk_jp-regular.otf";
		static constexpr std::string_view iconFontName = "Font Awesome 5";

		static constexpr std::array<f32, 3> textFontSizes = { 16.0f, (16.0f * 2.0f), (16.0f * 1.25f) };
		static constexpr f32 iconFontSize = (textFontSizes[0] - 2.0f);

		static constexpr f32 iconMinAdvanceX = 13.0f;
	};
}

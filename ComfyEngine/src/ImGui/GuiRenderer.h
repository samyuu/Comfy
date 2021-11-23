#pragma once
#include "Types.h"
#include "Gui.h"
#include "Window/ApplicationHost.h"

namespace ImGui
{
	class GuiRenderer : NonCopyable
	{
	public:
		// NOTE: Used for setting the initial window size and position
		static constexpr const char* MainDockSpaceID = "MainDockSpace";

		static constexpr const char* ConfigFileName = "settings_imgui.ini";
		static constexpr const char* ConfigFileNameDefault = "settings_imgui_def.ini";
		static constexpr const char* LogFileName = "imgui_log.txt";
		static constexpr std::string_view FontDirectoryName = "font";
		static constexpr std::string_view TextFontName = "Noto Sans CJK JP";
		static constexpr std::string_view TextFontFileName = "noto_sans_cjk_jp-regular.otf";
		static constexpr std::string_view IconFontName = "Font Awesome 5";

		static constexpr std::array<f32, 3> TextFontSizes = { 16.0f, (16.0f * 2.0f), (16.0f * 1.25f) };
		static constexpr f32 IconFontSize = (TextFontSizes[0] - 2.0f);

		static constexpr f32 IconMinAdvanceX = 13.0f;

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

		bool buildFullTextGlyphRange = false;
		bool fullFontRangeHasBeenRebuilt = false;
	};
}

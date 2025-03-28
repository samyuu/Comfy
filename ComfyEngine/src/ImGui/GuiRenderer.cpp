#include "GuiRenderer.h"
#include "GuiRendererGlyphRanges.h"
#include "Window/ApplicationHost.h"
#include "System/ComfyData.h"
#include "ImGui/Implementation/ComfyWin32.h"
#include "ImGui/Implementation/ComfyD3D11.h"
#include "FontIcons.h"

using namespace Comfy;

namespace ImGui
{
	LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

	GuiRenderer::GuiRenderer(ApplicationHost& host) : host(host)
	{
	}

	bool GuiRenderer::Initialize()
	{
		if (!CreateImGuiContext())
			return false;

		if (!SetStartupIOState())
			return false;

		LoadFontFiles();
		InitializeFonts();

		if (!SetComfyStyle())
			return false;

		if (!InitializeBackend())
			return false;

		return true;
	}

	void GuiRenderer::BeginFrame()
	{
		ComfyD3D11::NewFrame();
		ImGui_ImplWin32_NewFrame();
		NewFrame();
		UpdateExtendedState();
	}

	void GuiRenderer::EndFrame()
	{
		Render();

		// NOTE: Delaying the full glyph range font creation improves startup times making the program feel more responsive, even with the minor stutter on first use.
		//		 For debug builds the full glyph ranges takes relatively long to load and isn't really needed
#if !defined(COMFY_DEBUG)
		if (!fullFontRangeHasBeenRebuilt && Gui::GetFont()->MissingGlyphEncountered)
		{
			buildFullTextGlyphRange = true;
			InitializeFonts();
			GetIO().Fonts->Build();
			fullFontRangeHasBeenRebuilt = true;
		}
#endif

		ComfyD3D11::RenderDrawData(GetDrawData());

		const auto& io = GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			UpdatePlatformWindows();
			RenderPlatformWindowsDefault();
		}
	}

	void GuiRenderer::Dispose()
	{
		ComfyD3D11::Shutdown();
		ImGui_ImplWin32_Shutdown();
		DestroyContext();
	}

	bool GuiRenderer::IsAnyViewportFocused() const
	{
		return ImGui_ImplWin32_IsAnyViewportFocused();
	}

	bool GuiRenderer::CreateImGuiContext()
	{
		if (CreateContext() == nullptr)
			return false;

		return true;
	}

	bool GuiRenderer::SetStartupIOState()
	{
		auto& io = GetIO();
		io.IniFilename = ConfigFileName;
		io.LogFilename = LogFileName;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

#if 0 // TODO: Not yet properly supported by comfy gui widgets
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
#endif

		io.KeyRepeatDelay = 0.275f;
		io.KeyRepeatRate = 0.050f;

		io.ConfigDockingNoSplit = false;
		io.ConfigDockingAlwaysTabBar = true;
		io.ConfigDockingTransparentPayload = false;

		// NOTE: Was originally enabled but causes window focus issues when spawning a file dialog from within a (technically separate win32 window) popup
		io.ConfigViewportsNoAutoMerge = false;
		// NOTE: Only makes sense for decorated windows but even then font icons result in "?" symbols everywhere
		io.ConfigViewportsNoTaskBarIcon = true;
		// NOTE: Needs to match the auto-merge inverse to not cause any ugly window popping
		io.ConfigViewportsNoDecoration = !io.ConfigViewportsNoAutoMerge;
		io.ConfigViewportsNoDefaultParent = false;

		io.ConfigWindowsMoveFromTitleBarOnly = true;

		return true;
	}

	bool GuiRenderer::LoadFontFiles()
	{
		const auto fontDirectory = System::Data.FindDirectory(FontDirectoryName);
		if (fontDirectory == nullptr)
			return false;

		const auto textFontEntry = System::Data.FindFileInDirectory(*fontDirectory, TextFontFileName);
		const auto iconFontEntry = System::Data.FindFileInDirectory(*fontDirectory, FONT_ICON_FILE_NAME_FAS);

		if (textFontEntry == nullptr || iconFontEntry == nullptr)
			return false;

		textFontFileSize = textFontEntry->Size;
		iconFontFileSize = iconFontEntry->Size;
		combinedFontFileContent = std::make_unique<u8[]>(textFontFileSize + iconFontFileSize);

		u8* textFontFileContent = combinedFontFileContent.get();
		u8* iconFontFileContent = combinedFontFileContent.get() + textFontFileSize;

		if (!System::Data.ReadFileIntoBuffer(textFontEntry, textFontFileContent) || !System::Data.ReadFileIntoBuffer(iconFontEntry, iconFontFileContent))
		{
			combinedFontFileContent = nullptr;
			return false;
		}

		return true;
	}

	bool GuiRenderer::InitializeFonts()
	{
		auto& ioFonts = *GetIO().Fonts;
		ioFonts.Flags |= ImFontAtlasFlags_NoMouseCursors;

		if (combinedFontFileContent == nullptr || textFontFileSize == 0 || iconFontFileSize == 0)
			return false;

		if (!ioFonts.Fonts.empty())
			ioFonts.Clear();

		ImFontConfig textFontConfig = {};
		textFontConfig.FontDataOwnedByAtlas = false;
		memcpy(textFontConfig.Name, TextFontName.data(), TextFontName.size());

		ImFontConfig iconFontConfig = {};
		iconFontConfig.FontDataOwnedByAtlas = false;
		iconFontConfig.GlyphMinAdvanceX = IconMinAdvanceX;
		iconFontConfig.MergeMode = true;
		memcpy(iconFontConfig.Name, IconFontName.data(), IconFontName.size());

		u8* textFontFileContent = combinedFontFileContent.get();
		u8* iconFontFileContent = combinedFontFileContent.get() + textFontFileSize;

		if (ioFonts.AddFontFromMemoryTTF(textFontFileContent, static_cast<int>(textFontFileSize), TextFontSizes[0], &textFontConfig, GetTextGlyphRange()) == nullptr)
			return false;
		if (ioFonts.AddFontFromMemoryTTF(iconFontFileContent, static_cast<int>(iconFontFileSize), IconFontSize, &iconFontConfig, GetIconGlyphRange()) == nullptr)
			return false;

#if 0 // NOTE: Additional bold fonts for fancy formatting, not needed for now
		ImFontConfig boldFontConfig = {};
		boldFontConfig.FontDataOwnedByAtlas = false;
		memcpy(boldFontConfig.Name, TextFontName.data(), TextFontName.size());

		if (ioFonts.AddFontFromMemoryTTF(textFontFileContent, static_cast<int>(textFontFileSize), TextFontSizes[1], &boldFontConfig) == nullptr)
			return false;
		if (ioFonts.AddFontFromMemoryTTF(textFontFileContent, static_cast<int>(textFontFileSize), TextFontSizes[2], &boldFontConfig) == nullptr)
			return false;
#endif

		return true;
	}

	bool GuiRenderer::SetComfyStyle()
	{
		StyleComfy();
		return true;
	}

	bool GuiRenderer::InitializeBackend()
	{
		if (!ImGui_ImplWin32_Init(host.GetWindowHandle()))
			return false;

		//if (!ImGui_ImplDX11_Init())
		//	return false;

		//if (!ComfyWin32::Initialize(host.GetWindowHandle()))
		//	return false;

		if (!ComfyD3D11::Initialize())
			return false;

		host.RegisterWindowProcCallback(ImGui_ImplWin32_WndProcHandler);
		return true;
	}

	const ImWchar* GuiRenderer::GetTextGlyphRange() const
	{
		return buildFullTextGlyphRange ? TextFontGlyphRanges : GetIO().Fonts->GetGlyphRangesDefault();
	}

	const ImWchar* GuiRenderer::GetIconGlyphRange() const
	{
		static constexpr ImWchar iconFontGlyphRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		return iconFontGlyphRange;
	}
}

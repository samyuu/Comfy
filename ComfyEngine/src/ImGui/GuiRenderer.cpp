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

		LoadInitializeFontFiles();

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
		io.IniFilename = configFileName;
		io.LogFilename = logFileName;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

#if 0 // TODO: Not yet properly supported by comfy gui widgets
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavNoCaptureKeyboard;
#endif

#if 1
		io.ConfigViewportsNoAutoMerge = true;
		io.ConfigViewportsNoTaskBarIcon = false;
		io.ConfigViewportsNoDecoration = false;
		io.ConfigViewportsNoDefaultParent = false;
		io.ConfigDockingWithShift = false;
#else
		io.ConfigViewportsNoDecoration = true;
		io.ConfigDockingWithShift = false;
#endif

		io.KeyRepeatDelay = 0.250f;
		io.KeyRepeatRate = 0.050f;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigDockingTabBarOnSingleWindows = true;

		if (preLoadImGuiConfig)
		{
			LoadIniSettingsFromDisk(configFileName);

			if (restoreConfigWindowSize)
			{
				const ImGuiID mainDockspaceID = ImHashStr(MainDockSpaceID, 0);
				const ImGuiWindowSettings* settings = FindWindowSettings(mainDockspaceID);

				if (settings != nullptr)
				{
					host.SetWindowPosition(vec2(settings->Pos));
					host.SetWindowSize(vec2(settings->Size));
				}
			}
		}

		return true;
	}

	bool GuiRenderer::LoadInitializeFontFiles()
	{
		const auto fontDirectory = System::Data.FindDirectory(fontDirectoryName);
		if (fontDirectory == nullptr)
			return false;

		const auto textFontEntry = System::Data.FindFileInDirectory(*fontDirectory, textFontFileName);
		const auto iconFontEntry = System::Data.FindFileInDirectory(*fontDirectory, FONT_ICON_FILE_NAME_FAS);

		if (textFontEntry == nullptr || iconFontEntry == nullptr)
			return false;

		void* textFontFileContent = IM_ALLOC(textFontEntry->Size);
		void* iconFontFileContent = IM_ALLOC(iconFontEntry->Size);

		if (!System::Data.ReadFileIntoBuffer(textFontEntry, textFontFileContent) || !System::Data.ReadFileIntoBuffer(iconFontEntry, iconFontFileContent))
		{
			IM_FREE(textFontFileContent);
			IM_FREE(iconFontFileContent);
			return false;
		}

		auto& ioFonts = *GetIO().Fonts;
		ioFonts.Flags |= ImFontAtlasFlags_NoMouseCursors;

		ImFontConfig textFontConfig = {};
		textFontConfig.FontDataOwnedByAtlas = true;
		memcpy(textFontConfig.Name, textFontName.data(), textFontName.size());

		ImFontConfig iconFontConfig = {};
		iconFontConfig.FontDataOwnedByAtlas = true;
		iconFontConfig.GlyphMinAdvanceX = iconMinAdvanceX;
		iconFontConfig.MergeMode = true;
		memcpy(iconFontConfig.Name, iconFontName.data(), iconFontName.size());

		if (ioFonts.AddFontFromMemoryTTF(textFontFileContent, static_cast<int>(textFontEntry->Size), textFontSize, &textFontConfig, GetTextGlyphRange()) == nullptr)
			return false;

		if (ioFonts.AddFontFromMemoryTTF(iconFontFileContent, static_cast<int>(iconFontEntry->Size), iconFontSize, &iconFontConfig, GetIconGlyphRange()) == nullptr)
			return false;

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
#if COMFY_DEBUG && 1 // NOTE: Greatly improve debug build startup times
		return GetIO().Fonts->GetGlyphRangesDefault();
		// return GetIO().Fonts->GetGlyphRangesJapanese();
#endif /* COMFY_DEBUG */

		// TODO: Eventually replace with dynamic runtime glyph building or even loading a prerendered (BC7 compressed) font texture
		return TextFontGlyphRanges;
	}

	const ImWchar* GuiRenderer::GetIconGlyphRange() const
	{
		static constexpr ImWchar iconFontGlyphRange[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		return iconFontGlyphRange;
	}
}

#include "GuiRenderer.h"
#include "Core/ComfyData.h"
#include "FileSystem/FileHelper.h"
#include "ImGui/Implementation/ImGui_Impl.h"
#include "ImGui/Implementation/ImGui_Impl_Renderer.h"
#include "FontIcons.h"

namespace ImGui
{
	GuiRenderer::GuiRenderer(ApplicationHost& host)
		: host(host), iconFontGlyphRange { ICON_MIN_FA, ICON_MAX_FA, 0 }
	{
	}

	GuiRenderer::~GuiRenderer()
	{
	}

	bool GuiRenderer::Initialize()
	{
		if (!InitializeCreateContext())
			return false;

		if (!InitializeSetStartupIoState())
			return false;

		if (!InitializeLoadFontData())
			return false;

		if (!InitializeSetStyle())
			return false;

		if (!InitializeBackend())
			return false;

		return true;
	}

	void GuiRenderer::BeginFrame()
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		NewFrame();
		UpdateExtendedState();
	}

	void GuiRenderer::EndFrame()
	{
		Render();
		ImGui_ImplDX11_RenderDrawData(GetDrawData());

		const auto& io = GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			UpdatePlatformWindows();
			RenderPlatformWindowsDefault();
		}
	}

	void GuiRenderer::Dispose()
	{
		ImGui_ImplDX11_Shutdown();
		ImGui_ImplWin32_Shutdown();
		DestroyContext();
	}

	bool GuiRenderer::InitializeCreateContext()
	{
		if (CreateContext() == nullptr)
			return false;

		return true;
	}

	bool GuiRenderer::InitializeSetStartupIoState()
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

		io.ConfigDockingWithShift = false;
		io.ConfigViewportsNoDecoration = true;
		io.KeyRepeatDelay = 0.250f;
		io.KeyRepeatRate = 0.050f;
		io.ConfigWindowsMoveFromTitleBarOnly = true;
		io.ConfigDockingTabBarOnSingleWindows = true;

		return true;
	}

	bool GuiRenderer::InitializeLoadFontData()
	{
		const auto fontDirectory = ComfyData->FindDirectory(fontDirectoryName);
		if (fontDirectory == nullptr)
			return false;

		const auto& io = GetIO();
		if (const auto textFontEntry = ComfyData->FindFileInDirectory(fontDirectory, fontFileName); textFontEntry != nullptr)
		{
			void* fileContent = IM_ALLOC(textFontEntry->Size);
			ComfyData->ReadEntryIntoBuffer(textFontEntry, fileContent);

			if (io.Fonts->AddFontFromMemoryTTF(fileContent, static_cast<int>(textFontEntry->Size), fontSize, nullptr, GetFontGlyphRange()) == nullptr)
				return false;
		}
		else
		{
			return false;
		}

		if (const auto iconFontEntry = ComfyData->FindFileInDirectory(fontDirectory, FONT_ICON_FILE_NAME_FAS); iconFontEntry != nullptr)
		{
			void* fileContent = IM_ALLOC(iconFontEntry->Size);
			ComfyData->ReadEntryIntoBuffer(iconFontEntry, fileContent);

			const ImFontConfig config = GetIconFontConfig();
			if (io.Fonts->AddFontFromMemoryTTF(fileContent, static_cast<int>(iconFontEntry->Size), iconFontSize, &config, GetIconGlyphRange()) == nullptr)
				return false;
		}
		else
		{
			return false;
		}

		return true;
	}

	bool GuiRenderer::InitializeSetStyle()
	{
		StyleComfy();
		return true;
	}

	bool GuiRenderer::InitializeBackend()
	{
		if (!ImGui_ImplWin32_Init(host.GetWindow()))
			return false;

		if (!ImGui_ImplDX11_Init())
			return false;

		extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
		host.RegisterWindowProcCallback(ImGui_ImplWin32_WndProcHandler);

		return true;
	}

	const ImWchar* GuiRenderer::GetFontGlyphRange() const
	{
		return GetIO().Fonts->GetGlyphRangesJapanese();
	}

	const ImWchar* GuiRenderer::GetIconGlyphRange() const
	{
		return iconFontGlyphRange;
	}

	ImFontConfig GuiRenderer::GetIconFontConfig() const
	{
		ImFontConfig config = {};
		config.GlyphMinAdvanceX = iconMinAdvanceX;
		config.MergeMode = true;
		return config;
	}
}
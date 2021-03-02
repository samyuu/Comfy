#include "PVScriptImportWindow.h"
#include "ImGui/Extensions/ImGuiExtensions.h"
#include "IO/File.h"
#include "IO/Path.h"

namespace Comfy::Studio::Editor
{
	void PVScriptImportWindow::Gui()
	{
	}

	void PVScriptImportWindow::SetScriptFilePath(std::string_view path)
	{
		scriptPath = IO::Path::Normalize(path);
	}

	bool PVScriptImportWindow::GetAndClearCloseRequestThisFrame()
	{
		const bool result = closeWindowThisFrame;
		closeWindowThisFrame = false;
		return result;
	}

	std::unique_ptr<Chart> PVScriptImportWindow::TryMoveImportedChartBeforeClosing()
	{
		// TODO: Reset internal state (?)
		return std::move(importedChart);
	}
}

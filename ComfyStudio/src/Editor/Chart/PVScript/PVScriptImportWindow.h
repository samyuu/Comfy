#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include "Editor/Chart/Chart.h"
#include "Script/PVScript.h"

namespace Comfy::Studio::Editor
{
	class PVScriptImportWindow : NonCopyable
	{
	public:
		PVScriptImportWindow() = default;
		~PVScriptImportWindow() = default;

	public:
		void Gui();

		void SetScriptFilePath(std::string_view path);

		bool GetAndClearCloseRequestThisFrame();
		std::unique_ptr<Chart> TryMoveImportedChartBeforeClosing();

	private:
		std::string scriptPath, songPath;

		std::unique_ptr<PVScript> loadedScript = nullptr;

		std::unique_ptr<Chart> importedChart = nullptr;
		bool closeWindowThisFrame = false;
	};
}

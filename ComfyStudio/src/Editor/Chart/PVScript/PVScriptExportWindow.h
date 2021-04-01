#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "ImGui/Gui.h"
#include "Editor/Chart/Chart.h"
#include "Script/PVScript.h"
#include "PVScriptUtil.h"

namespace Comfy::Studio::Editor
{
	class PVScriptExportWindow : NonCopyable
	{
	public:
		PVScriptExportWindow() = default;
		~PVScriptExportWindow() = default;

	public:
		void Gui();
		bool GetAndClearCloseRequestThisFrame();

	private:
		bool closeWindowThisFrame = false;
		bool thisFrameAnyItemActive = false, lastFrameAnyItemActive = false;
	};
}

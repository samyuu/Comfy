#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class PresetWindow : NonCopyable
	{
	public:
		PresetWindow(Undo::UndoManager& undoManager);
		~PresetWindow() = default;

	public:
		void Gui(Chart& chart);

	private:
		Undo::UndoManager& undoManager;
	};
}

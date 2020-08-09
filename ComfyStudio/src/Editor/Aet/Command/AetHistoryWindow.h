#pragma once
#include "Types.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	// TODO: Turn into generic HistoryWindow and use for both AetEditor and ChartEditor, refactor undo code...
	class AetHistoryWindow
	{
	public:
		AetHistoryWindow(Undo::UndoManager& undoManager);
		~AetHistoryWindow() = default;

	public:
		bool Gui();

	private:
		Undo::UndoManager& undoManager;
	};
}

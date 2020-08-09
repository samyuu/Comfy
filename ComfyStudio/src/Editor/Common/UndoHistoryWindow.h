#pragma once
#include "Types.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class UndoHistoryWindow
	{
	public:
		UndoHistoryWindow(Undo::UndoManager& undoManager);
		~UndoHistoryWindow() = default;

	public:
		bool Gui();

	private:
		Undo::UndoManager& undoManager;
	};
}

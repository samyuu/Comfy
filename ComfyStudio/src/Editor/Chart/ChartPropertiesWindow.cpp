#include "ChartPropertiesWindow.h"
#include "ChartEditor.h"

namespace Comfy::Studio::Editor
{
	ChartPropertiesWindow::ChartPropertiesWindow(ChartEditor& parent, Undo::UndoManager& undoManager) : chartEditor(parent), undoManager(undoManager)
	{
	}

	void ChartPropertiesWindow::Gui(Chart& chart)
	{
	}
}

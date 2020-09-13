#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor;

	class ChartPropertiesWindow : NonCopyable
	{
	public:
		ChartPropertiesWindow(ChartEditor& parent, Undo::UndoManager& undoManager);
		~ChartPropertiesWindow() = default;

	public:
		void Gui(Chart& chart);

	private:
		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;

	private:
	};
}

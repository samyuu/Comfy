#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class TargetInspector : NonCopyable
	{
	public:
		TargetInspector(Undo::UndoManager& undoManager);
		~TargetInspector() = default;

	public:
		void Gui(Chart& chart);

	private:
		Undo::UndoManager& undoManager;
	};
}

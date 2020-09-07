#include "TargetInspector.h"
#include "TargetPropertyRules.h"
#include "ImGui/Gui.h"
#include "ImGui/Extensions/PropertyEditor.h"

namespace Comfy::Studio::Editor
{
	TargetInspector::TargetInspector(Undo::UndoManager& undoManager) : undoManager(undoManager)
	{
	}

	void TargetInspector::Gui(Chart& chart)
	{
	}
}

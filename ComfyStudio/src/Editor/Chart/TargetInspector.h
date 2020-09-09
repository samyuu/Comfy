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
		std::string_view FormatSelectedTargetsValueBuffer(const Chart& chart);

		void GuiSelectedTargets(Chart& chart);
		void PropertyGui(Chart& chart, std::string_view label, TargetPropertyType property, f32 dragSpeed = 1.0f);

		i32 GetSelectedTargetIndex(const Chart& chart, const TimelineTarget* selectedTarget) const;

	private:
		Undo::UndoManager& undoManager;

		std::array<char, 64> selectedTargetsValueBuffer;

		// NOTE: All of these are only valid between the begin and end of Gui()
		std::vector<TimelineTarget*> selectedTargets;

		TimelineTarget* frontSelectedTarget;
		bool frontSelectedHasProperties;
		TargetProperties frontSelectedProperties;
	};
}

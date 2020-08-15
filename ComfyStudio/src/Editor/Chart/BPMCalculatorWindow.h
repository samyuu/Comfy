#pragma once
#include "Types.h"
#include "Chart.h"
#include "BPMTapCalculator.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class BPMCalculatorWindow
	{
	public:
		BPMCalculatorWindow(Undo::UndoManager& undoManager);
		~BPMCalculatorWindow() = default;

	public:
		void Gui(Chart& chart, TimeSpan cursorBPMTime);

	private:
		void ExecuteUpdateTempoChangeBPM(Chart& chart, TimeSpan cursorBPMTime, Tempo updatedTempo) const;

	private:
		Undo::UndoManager& undoManager;

		bool applyTapToTempoMap = true;
		BPMTapCalculator bpmCalculator = {};
	};
}

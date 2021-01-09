#pragma once
#include "Types.h"
#include "Chart.h"
#include "BPMTapCalculator.h"
#include "Undo/Undo.h"
#include "Timeline/TimelineMetronome.h"

namespace Comfy::Studio::Editor
{
	class BPMCalculatorWindow
	{
	public:
		BPMCalculatorWindow(Undo::UndoManager& undoManager);
		~BPMCalculatorWindow() = default;

	public:
		void Gui(Chart& chart, TimeSpan cursorBPMTime, TimelineMetronome& metronome);

	private:
		void PlayTapSoundIfEnabled(TimelineMetronome& metronome) const;
		void ExecuteUpdateTempoChangeBPM(Chart& chart, TimeSpan cursorBPMTime, Tempo updatedTempo) const;

	private:
		Undo::UndoManager& undoManager;

		bool applyTapToTempoMap = false;
		bool playTapSound = true;
		bool useBarTapSound = false;

		BPMTapCalculator bpmCalculator = {};
	};
}

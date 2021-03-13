#pragma once
#include "Types.h"
#include "Chart.h"
#include "BPMTapCalculator.h"
#include "Timeline/TimelineMetronome.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	enum class BPMTapSoundType : u8
	{
		None,
		MetronomeBeat,
		MetronomeBar,
		Count,
	};

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
		BPMTapCalculator bpmCalculator = {};
	};
}

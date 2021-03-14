#pragma once
#include "Types.h"
#include "Chart.h"
#include "BPMTapCalculator.h"
#include "Timeline/TimelineMetronome.h"
#include "Editor/Common/ButtonSoundController.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	enum class BPMTapSoundType : u8
	{
		None,
		MetronomeBeat,
		MetronomeBar,
		ButtonSound,
		SlideSound,
		Count,
	};

	constexpr std::array<const char*, EnumCount<BPMTapSoundType>()> BPMTapSoundTypeNames =
	{
		"None",
		"Metronome Beat",
		"Metronome Bar",
		"Button Sound",
		"Slide Sound",
	};

	class BPMCalculatorWindow
	{
	public:
		BPMCalculatorWindow(Undo::UndoManager& undoManager);
		~BPMCalculatorWindow() = default;

	public:
		void Gui(Chart& chart, TimeSpan cursorBPMTime, TimelineMetronome& metronome, ButtonSoundController& buttonSoundController);

	private:
		void TryPlayTapSound(TimelineMetronome& metronome, ButtonSoundController& buttonSoundController) const;
		void ExecuteUpdateTempoChangeBPM(Chart& chart, TimeSpan cursorBPMTime, Tempo updatedTempo) const;

	private:
		Undo::UndoManager& undoManager;
		BPMTapCalculator bpmCalculator = {};
	};
}

#pragma once
#include "Chart.h"
#include "Timeline/TargetTimeline.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class SyncWindow
	{
	public:
		SyncWindow(Undo::UndoManager& undoManager);
		~SyncWindow() = default;

	public:
		void OnFirstFrame();
		void Gui(Chart& chart, TargetTimeline& timeline);

	private:
		Undo::UndoManager& undoManager;

		Tempo newTempo = TempoChange::DefaultTempo;
		TimeSignature newSignature = TempoChange::DefaultSignature;
	};
}

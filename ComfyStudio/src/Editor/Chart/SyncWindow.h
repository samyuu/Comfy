#pragma once
#include "Types.h"
#include "Chart.h"
#include "Timeline/TargetTimeline.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class SyncWindow : NonCopyable
	{
	public:
		SyncWindow(Undo::UndoManager& undoManager);
		~SyncWindow() = default;

	public:
		void Gui(Chart& chart, TargetTimeline& timeline);

	private:
		Undo::UndoManager& undoManager;

		BeatTick thisFrameCursorTick, lastFrameCursorTick;
	};
}

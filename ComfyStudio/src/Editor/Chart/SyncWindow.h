#pragma once
#include "Chart.h"
#include "Timeline/TargetTimeline.h"

namespace Comfy::Studio::Editor
{
	class SyncWindow
	{
	public:
		SyncWindow() = default;
		~SyncWindow() = default;

	public:
		void OnFirstFrame();
		void Gui(Chart& chart, TargetTimeline& timeline);

	private:
		Tempo newTempo = TempoChange::DefaultTempo;
	};
}

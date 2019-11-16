#pragma once
#include "Chart.h"
#include "Timeline/TargetTimeline.h"

namespace Editor
{
	class SyncWindow
	{
	public:
		SyncWindow();
		~SyncWindow();

		void Initialize();
		void DrawGui(Chart* chart, TargetTimeline* timeline);

	private:
		Tempo newTempo = TempoChange::DefaultTempo;
	};
}
#pragma once
#include "ImGui/Gui.h"

namespace Editor
{
	class TimelineBaseRegions
	{
	public:
		inline const ImRect& GetTimelineRegion() const { return timelineRegion; };
		inline const ImRect& GetInfoColumnHeaderRegion() const { return infoColumnHeaderRegion; };
		inline const ImRect& GetInfoColumnRegion() const { return infoColumnRegion; };
		inline const ImRect& GetTimelineBaseRegion() const { return timelineBaseRegion; };
		inline const ImRect& GetTempoMapRegion() const { return tempoMapRegion; };
		inline const ImRect& GetTimelineHeaderRegion() const { return timelineHeaderRegion; };
		inline const ImRect& GetTimelineContentRegion() const { return timelineContentRegion; };

	protected:
		ImRect timelineRegion;
		ImRect infoColumnHeaderRegion;
		ImRect infoColumnRegion;
		ImRect timelineBaseRegion;
		ImRect tempoMapRegion;
		ImRect timelineHeaderRegion;
		ImRect timelineContentRegion;

		virtual void UpdateTimelineRegions() = 0;
	};
}
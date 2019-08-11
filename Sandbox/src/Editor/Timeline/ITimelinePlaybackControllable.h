#pragma once

namespace Editor
{
	class ITimelinePlaybackControllable
	{
	public:
		virtual bool GetIsPlayback() const = 0;

	protected:
		virtual void PausePlayback() = 0;
		virtual void ResumePlayback() = 0;
		virtual void StopPlayback() = 0;
	};
}
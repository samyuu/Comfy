#pragma once

namespace Comfy::Studio::Editor
{
	class ITimelinePlaybackControllable
	{
	public:
		virtual ~ITimelinePlaybackControllable() = default;

		virtual bool GetIsPlayback() const = 0;

	protected:
		virtual void PausePlayback() = 0;
		virtual void ResumePlayback() = 0;
		virtual void StopPlayback() = 0;
	};
}

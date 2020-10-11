#pragma once
#include "IAudioBackend.h"

namespace Comfy::Audio
{
	// TODO: Busy spin stopwatch callback to always advance voices then use as fallback when the requested backend failed
	class NullOutBackend : public IAudioBackend, NonCopyable {};

	// TODO: Implement
	class ASIOBackend : public IAudioBackend, NonCopyable {};

	class WASAPIBackend : public IAudioBackend, NonCopyable
	{
	public:
		WASAPIBackend();
		~WASAPIBackend();

	public:
		bool OpenStartStream(const StreamParameters& param, RenderCallbackFunc callback) override;
		bool StopCloseStream() override;

	public:
		bool IsOpenRunning() const override;

	private:
		struct Impl;
		std::unique_ptr<Impl> impl;
	};
}

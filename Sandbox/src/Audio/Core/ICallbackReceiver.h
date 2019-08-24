#pragma once

namespace Audio
{
	class ICallbackReceiver
	{
	public:
		virtual void OnAudioCallback() = 0;
	};
}
#pragma once

namespace Comfy::Audio
{
	class ICallbackReceiver
	{
	public:
		virtual void OnAudioCallback() = 0;
	};
}

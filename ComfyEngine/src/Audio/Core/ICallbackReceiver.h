#pragma once

namespace Comfy::Audio
{
	class ICallbackReceiver
	{
	public:
		virtual ~ICallbackReceiver() = default;

		virtual void OnAudioCallback() = 0;
	};
}

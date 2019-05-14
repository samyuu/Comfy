#pragma once

class ICallbackReceiver
{
public:
	virtual void OnAudioCallback() = 0;
};
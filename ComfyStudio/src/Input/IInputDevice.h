#pragma once

class IInputDevice
{
public:
	virtual bool PollInput() = 0;
};

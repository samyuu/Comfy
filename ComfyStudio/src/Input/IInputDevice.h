#pragma once

namespace Comfy
{
	class IInputDevice
	{
	public:
		virtual bool PollInput() = 0;
	};
}

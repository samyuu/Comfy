#pragma once

namespace Comfy::Input
{
	class IInputDevice
	{
	public:
		virtual bool PollInput() = 0;
	};
}

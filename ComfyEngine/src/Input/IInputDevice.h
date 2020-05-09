#pragma once

namespace Comfy::Input
{
	class IInputDevice
	{
	public:
		virtual ~IInputDevice() = default;

		virtual bool PollInput() = 0;
	};
}

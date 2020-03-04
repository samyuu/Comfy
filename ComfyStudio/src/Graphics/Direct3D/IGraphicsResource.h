#pragma once
#include "Types.h"

namespace Comfy::Graphics
{
	class IGraphicsResource : NonCopyable
	{
	protected:
		IGraphicsResource() = default;
		virtual ~IGraphicsResource() = default;

	protected:
	};
}

#pragma once
#include "Types.h"

namespace Comfy::Graphics
{
	class ID3DGraphicsResource : NonCopyable
	{
	protected:
		ID3DGraphicsResource() = default;
		virtual ~ID3DGraphicsResource() = default;

	protected:
	};
}

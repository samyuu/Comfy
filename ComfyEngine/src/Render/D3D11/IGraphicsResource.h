#pragma once
#include "Types.h"

namespace Comfy::Graphics::D3D11
{
	class IGraphicsResource : NonCopyable
	{
	protected:
		IGraphicsResource() = default;
		virtual ~IGraphicsResource() = default;

	protected:
	};
}

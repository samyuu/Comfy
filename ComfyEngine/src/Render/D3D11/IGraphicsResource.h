#pragma once
#include "Types.h"

namespace Comfy::Render::D3D11
{
	class IGraphicsResource : NonCopyable
	{
	protected:
		IGraphicsResource() = default;
		virtual ~IGraphicsResource() = default;

	protected:
	};
}

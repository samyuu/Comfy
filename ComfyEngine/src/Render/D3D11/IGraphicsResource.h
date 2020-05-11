#pragma once
#include "Types.h"
#include "Graphics/GPUResource.h"

namespace Comfy::Render::D3D11
{
	class IGraphicsResource : public Graphics::GPUResource, NonCopyable
	{
	protected:
		IGraphicsResource() = default;
	public:
		virtual ~IGraphicsResource() = default;
	};
}

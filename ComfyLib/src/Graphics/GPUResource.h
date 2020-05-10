#pragma once

namespace Comfy::Graphics
{
	// NOTE: Opaque type for any GPU resource. Initialized and used internally by the Comfy::Graphics::Render library
	class GPUResource
	{
	public:
		virtual ~GPUResource() = default;
	};
}

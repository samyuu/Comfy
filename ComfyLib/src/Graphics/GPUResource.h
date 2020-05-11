#pragma once

namespace Comfy::Graphics
{
	// NOTE: Opaque type for any GPU resource. Initialized and used internally by the Comfy::Render library.
	//		 Should always be stored wrapped inside an std::unique_ptr, which to reupload should be set to nullptr
	class GPUResource
	{
	public:
		virtual ~GPUResource() = default;
	};
}

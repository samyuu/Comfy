#pragma once
#include "Types.h"

namespace Comfy::Graphics
{
	// NOTE: Opaque base type for any GPU resource
	class OpaqueGPUResource
	{
	public:
		virtual ~OpaqueGPUResource() = default;
	};

	// NOTE: Initialized and used internally by the Comfy::Render library
	struct InternallyManagedGPUResource
	{
		mutable std::unique_ptr<OpaqueGPUResource> Resource = nullptr;
		mutable bool RequestReupload = false;
		mutable bool DynamicResource = false;
	};
}

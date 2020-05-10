#pragma once
#include "Graphics/D3D11/State/OcclusionQuery.h"

namespace Comfy::Render::D3D11
{
	struct SunOcclusionData
	{
		OcclusionQuery OcclusionQuery = { "Renderer3D::SunOcclusionQuery" };
	};
}

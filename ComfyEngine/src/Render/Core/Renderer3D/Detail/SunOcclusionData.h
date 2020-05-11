#pragma once
#include "Render/D3D11/State/OcclusionQuery.h"

namespace Comfy::Render::Detail
{
	struct SunOcclusionData
	{
		D3D11::OcclusionQuery OcclusionQuery = { "Renderer3D::SunOcclusionQuery" };
	};
}

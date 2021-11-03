#pragma once
#include "Render/D3D11/D3D11State.h"

namespace Comfy::Render::Detail
{
	struct SunOcclusionData
	{
		// NOTE: Sun covered by opaque geometry
		D3D11OcclusionQuery OcclusionQuery = { GlobalD3D11, "Renderer3D::SunOcclusionQuery" };

		// TODO: Array of queries to detect the sun being at the edge of the screen
		// D3D11OcclusionQuery OffScreenOcclusionQuery = { GlobalD3D11, "Renderer3D::SunOffScreenOcclusionQuery" };

		// NOTE: Sun without depth testing to calculate covered percentage
		D3D11OcclusionQuery NoDepthOcclusionQuery = { GlobalD3D11, "Renderer3D::SunNoDepthOcclusionQuery" };
	};
}

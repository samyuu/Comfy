#pragma once
#include "Render/D3D11/State/OcclusionQuery.h"

namespace Comfy::Render::Detail
{
	struct SunOcclusionData
	{
		// NOTE: Sun covered by opaque geometry
		D3D11::OcclusionQuery OcclusionQuery = { "Renderer3D::SunOcclusionQuery" };

		// TODO: Array of queries to detect the sun being at the edge of the screen
		// D3D11::OcclusionQuery OffScreenOcclusionQuery = { "Renderer3D::SunOffScreenOcclusionQuery" };

		// NOTE: Sun without depth testing to calculate covered percentage
		D3D11::OcclusionQuery NoDepthOcclusionQuery = { "Renderer3D::SunNoDepthOcclusionQuery" };
	};
}

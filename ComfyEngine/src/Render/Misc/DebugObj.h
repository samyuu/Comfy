#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Render
{
	constexpr int MinSphereMeshDetailLevel = 0;
	constexpr int MaxSphereMeshDetailLevel = 6;

	int GetSphereMeshDetailLevelForRadius(const Graphics::Sphere& sphere);

	std::unique_ptr<Graphics::Obj> GenerateUploadDebugBoxObj(const Graphics::Box& box, const vec4& color);
	std::unique_ptr<Graphics::Obj> GenerateUploadDebugSphereObj(const Graphics::Sphere& sphere, const vec4& color, int detailLevel = -1);
	std::unique_ptr<Graphics::Obj> GenerateUploadMaterialTestSphereObj();
}

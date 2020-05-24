#pragma once
#include "Types.h"
#include "Graphics/Auth3D/ObjSet.h"

namespace Comfy::Graphics
{
	constexpr int MinSphereMeshDetailLevel = 0;
	constexpr int MaxSphereMeshDetailLevel = 6;

	int GetSphereMeshDetailLevelForRadius(const Sphere& sphere);

	std::unique_ptr<Obj> GenerateDebugBoxObj(const Box& box, const vec4& color);
	std::unique_ptr<Obj> GenerateDebugSphereObj(const Sphere& sphere, const vec4& color, int detailLevel = -1);
	std::unique_ptr<Obj> GenerateMaterialTestSphereObj();
}

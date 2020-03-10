#pragma once
#include "Types.h"
#include "ObjSet.h"

namespace Comfy::Graphics
{
	constexpr int MinSphereMeshDetailLevel = 0;
	constexpr int MaxSphereMeshDetailLevel = 6;

	int GetSphereMeshDetailLevelForRadius(const Sphere& sphere);

	UniquePtr<Obj> GenerateUploadDebugBoxObj(const Box& box, const vec4& color);
	UniquePtr<Obj> GenerateUploadDebugSphereObj(const Sphere& sphere, const vec4& color, int detailLevel = -1);
}

#pragma once
#include "A3D.h"
#include "../Transform.h"

namespace Comfy::Graphics
{
	// TODO: Rename to A3D::Util and turn into a namespace
	class A3DMgr
	{
	public:
		static f32 Interpolate(A3DTangentType type, const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame);
		static f32 InterpolateLinear(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame);
		static f32 InterpolateHermite(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame);
		static f32 InterpolateHold(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame);

		static std::array<const A3DKeyFrame*, 2> FindStartEndKeyFramesAt(const A3DProperty1D& property, frame_t frame);

		static f32 GetValueAt(const A3DProperty1D& property, frame_t frame);
		static vec3 GetValueAt(const A3DProperty3D& property, frame_t frame);

		static f32 GetRotationAt(const A3DProperty1D& property, frame_t frame);
		static vec3 GetRotationAt(const A3DProperty3D& property, frame_t frame);

		static bool GetBool(f32 value);
		static bool GetBoolAt(const A3DProperty1D& property, frame_t frame);

		static int GetInt(f32 value);
		static int GetIntAt(const A3DProperty1D& property, frame_t frame);

		static Transform GetTransformAt(const A3DTransform& transform, frame_t frame);
		static f32 GetFieldOfViewAt(const A3DCameraViewPoint& viewPoint, frame_t frame);
	};
}

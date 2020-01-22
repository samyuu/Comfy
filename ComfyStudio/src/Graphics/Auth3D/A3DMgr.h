#pragma once
#include "A3D.h"
#include "Transform.h"

namespace Graphics
{
	class A3DMgr
	{
	public:
		static float Interpolate(A3DInterpolationType type, const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame);
		static float InterpolateLinear(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame);
		static float InterpolateHermit(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame);
		static float InterpolateHold(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame);
		
		static std::array<const A3DKeyFrame*, 2> GetStartEndKeyFramesAt(const A3DProperty1D& property, frame_t frame);

		static float GetValueAt(const A3DProperty1D& property, frame_t frame);
		static vec3 GetValueAt(const A3DProperty3D& property, frame_t frame);
		
		static Transform GetTransformAt(const A3DTransform& transform, frame_t frame);
		
		static bool GetBool(float value);
		static bool GetBoolAt(const A3DProperty1D& property, frame_t frame);
		
		static float GetFieldOfViewAt(const A3DCameraViewPoint& viewPoint, frame_t frame);
	};
}

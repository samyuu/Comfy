#include "A3DMgr.h"

namespace Graphics
{
	float A3DMgr::Interpolate(A3DInterpolationType type, const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame)
	{
		if (start->Frame >= end->Frame)
			return start->Value;

		switch (type)
		{
		case A3DInterpolationType::Linear:
			return A3DMgr::InterpolateLinear(start, end, frame);

		case A3DInterpolationType::Hermit:
			return A3DMgr::InterpolateHermit(start, end, frame);

		case A3DInterpolationType::Hold:
			return A3DMgr::InterpolateHold(start, end, frame);

		default:
			return 0.0f;
		}
	}

	float A3DMgr::InterpolateLinear(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame)
	{
		const float range = end->Frame - start->Frame;
		const float t = (frame - start->Frame) / range;

		return ((1.0f - t) * start->Value) + (t * end->Value);
	}

	float A3DMgr::InterpolateHermit(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame)
	{
		const float range = end->Frame - start->Frame;
		const float t = (frame - start->Frame) / range;

		return ((((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start->Value)
			+ ((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end->Value))
			+ (((((t * t) * t) - ((t * t) * 2.0f)) + t) * (range * start->EndCurve)))
			+ ((((t * t) * t) - (t * t)) * (range * end->StartCurve));
	}

	float A3DMgr::InterpolateHold(const A3DKeyFrame* start, const A3DKeyFrame* end, frame_t frame)
	{
		return (frame >= end->Frame) ? end->Value : start->Value;
	}

	std::array<const A3DKeyFrame*, 2> A3DMgr::GetStartEndKeyFramesAt(const A3DProperty1D& property, frame_t frame)
	{
		const A3DKeyFrame* start = &property.Keys.front();
		const A3DKeyFrame* end = start;

		for (int i = 1; i < property.Keys.size(); i++)
		{
			end = &property.Keys[i];
			if (end->Frame >= frame)
				break;
			start = end;
		}

		return { start, end };
	}

	float A3DMgr::GetValueAt(const A3DProperty1D& property, frame_t frame)
	{
		if (property.Type == A3DInterpolationType::Static)
			return property.StaticValue;

		if (property.Type == A3DInterpolationType::None || property.Type >= A3DInterpolationType::Count)
			return 0.0f;

		if (property.Keys.empty())
			return 0.0f;

		auto& first = property.Keys.front();
		auto& last = property.Keys.back();

		if (property.Keys.size() == 1 || frame <= first.Frame)
			return first.Value;

		if (frame >= last.Frame)
			return last.Value;

		auto[start, end] = A3DMgr::GetStartEndKeyFramesAt(property, frame);
		return A3DMgr::Interpolate(property.Type, start, end, frame);
	}

	vec3 A3DMgr::GetValueAt(const A3DProperty3D& property, frame_t frame)
	{
		vec3 result;
		result.x = A3DMgr::GetValueAt(property.X, frame);
		result.y = A3DMgr::GetValueAt(property.Y, frame);
		result.z = A3DMgr::GetValueAt(property.Z, frame);
		return result;
	}

	Transform A3DMgr::GetTransformAt(const A3DTransform& transform, frame_t frame)
	{
		Transform result;
		result.Translation = A3DMgr::GetValueAt(transform.Translation, frame);
		result.Scale = A3DMgr::GetValueAt(transform.Scale, frame);
		result.Rotation = glm::degrees(A3DMgr::GetValueAt(transform.Rotation, frame));
		return result;
	}

	bool A3DMgr::GetVisibilityAt(const A3DTransform& transform, frame_t frame)
	{
		constexpr float threshold = 0.999999f;
		return GetValueAt(transform.Visibility, frame) >= threshold;
	}

	float A3DMgr::GetFieldOfViewAt(const A3DCameraViewPoint& viewPoint, frame_t frame)
	{
		const float fov = A3DMgr::GetValueAt(viewPoint.FieldOfView, frame);
		
		// NOTE: Could potentially be affected by PerspectiveCamera::AspectRatio
		const float aspectRatio = viewPoint.AspectRatio; 

		// TODO: Vertical FOV is not correct (?)
		const float result = (viewPoint.HorizontalFieldOfView) ?
			(glm::atan(glm::tan(fov * 0.5f) / aspectRatio) * 2.0f) :
			(glm::atan((((aspectRatio * 25.4f) * 0.5f) / fov)) * 2.0f);

		return result * (180.0f / glm::pi<float>());
	}
}

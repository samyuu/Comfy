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
}

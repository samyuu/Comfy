#include "A3DMgr.h"

namespace Comfy::Graphics
{
	f32 A3DMgr::Interpolate(A3DTangentType type, const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame)
	{
		if (start.Frame >= end.Frame)
			return start.Value;

		switch (type)
		{
		case A3DTangentType::Linear:
			return A3DMgr::InterpolateLinear(start, end, frame);

		case A3DTangentType::Hermit:
			return A3DMgr::InterpolateHermit(start, end, frame);

		case A3DTangentType::Hold:
			return A3DMgr::InterpolateHold(start, end, frame);

		default:
			return 0.0f;
		}
	}

	f32 A3DMgr::InterpolateLinear(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame)
	{
		const f32 range = end.Frame - start.Frame;
		const f32 t = (frame - start.Frame) / range;

		return ((1.0f - t) * start.Value) + (t * end.Value);
	}

	f32 A3DMgr::InterpolateHermit(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame)
	{
		const f32 range = end.Frame - start.Frame;
		const f32 t = (frame - start.Frame) / range;

		return ((((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start.Value)
			+ ((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end.Value))
			+ (((((t * t) * t) - ((t * t) * 2.0f)) + t) * (range * start.EndTangent)))
			+ ((((t * t) * t) - (t * t)) * (range * end.StartTangent));
	}

	f32 A3DMgr::InterpolateHold(const A3DKeyFrame& start, const A3DKeyFrame& end, frame_t frame)
	{
		return (frame >= end.Frame) ? end.Value : start.Value;
	}

	std::array<const A3DKeyFrame*, 2> A3DMgr::FindStartEndKeyFramesAt(const A3DProperty1D& property, frame_t frame)
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

	f32 A3DMgr::GetValueAt(const A3DProperty1D& property, frame_t frame)
	{
		if (property.Type == A3DTangentType::Static)
			return property.StaticValue;

		if (property.Type == A3DTangentType::None || property.Type >= A3DTangentType::Count)
			return 0.0f;

		if (property.Keys.empty())
			return 0.0f;

		auto& first = property.Keys.front();
		auto& last = property.Keys.back();

		if (property.Keys.size() == 1 || frame <= first.Frame)
			return first.Value;

		// TODO: Correctly implement all different types
		if (property.PostInfinity == A3DInfinityType::Repeat)
			frame = glm::mod(frame, last.Frame);

		if (frame > last.Frame)
			return last.Value;

		auto[start, end] = A3DMgr::FindStartEndKeyFramesAt(property, frame);
		return A3DMgr::Interpolate(property.Type, *start, *end, frame);
	}

	vec3 A3DMgr::GetValueAt(const A3DProperty3D& property, frame_t frame)
	{
		vec3 result;
		result.x = A3DMgr::GetValueAt(property.X, frame);
		result.y = A3DMgr::GetValueAt(property.Y, frame);
		result.z = A3DMgr::GetValueAt(property.Z, frame);
		return result;
	}

	f32 A3DMgr::GetRotationAt(const A3DProperty1D& property, frame_t frame)
	{
		return glm::degrees(GetValueAt(property, frame));
	}

	vec3 A3DMgr::GetRotationAt(const A3DProperty3D& property, frame_t frame)
	{
		// TODO: Rotation Interpolation, None, Euler, Quaternion Tangent Dependent, Quaternion Slerp, Quaternion Squad (?)
		return glm::degrees(GetValueAt(property, frame));
	}

	bool A3DMgr::GetBool(f32 value)
	{
		constexpr f32 threshold = 0.999f;
		return (value >= threshold);
	}

	bool A3DMgr::GetBoolAt(const A3DProperty1D& property, frame_t frame)
	{
		return GetBool(GetValueAt(property, frame));
	}

	int A3DMgr::GetInt(f32 value)
	{
		return static_cast<int>(glm::round(value));
	}

	int A3DMgr::GetIntAt(const A3DProperty1D& property, frame_t frame)
	{
		return GetInt(GetValueAt(property, frame));
	}

	Transform A3DMgr::GetTransformAt(const A3DTransform& transform, frame_t frame)
	{
		Transform result;
		result.Translation = A3DMgr::GetValueAt(transform.Translation, frame);
		result.Scale = A3DMgr::GetValueAt(transform.Scale, frame);
		result.Rotation = A3DMgr::GetRotationAt(transform.Rotation, frame);
		return result;
	}

	f32 A3DMgr::GetFieldOfViewAt(const A3DCameraViewPoint& viewPoint, frame_t frame)
	{
		const f32 fov = A3DMgr::GetValueAt(viewPoint.FieldOfView, frame);

		// NOTE: Could potentially be affected by PerspectiveCamera::AspectRatio
		const f32 aspectRatio = viewPoint.AspectRatio;

		// TODO: Vertical FOV is not correct (?)
		const f32 result = (viewPoint.HorizontalFieldOfView) ?
			(glm::atan(glm::tan(fov * 0.5f) / aspectRatio) * 2.0f) :
			(glm::atan((((aspectRatio * 25.4f) * 0.5f) / fov)) * 2.0f);

		return result * (180.0f / glm::pi<f32>());
	}
}

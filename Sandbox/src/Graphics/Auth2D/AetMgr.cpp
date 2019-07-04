#include "AetMgr.h"

namespace Auth2D
{
	float AetMgr::Interpolate(const std::vector<KeyFrame>& keyFrames, float frame)
	{
		if (keyFrames.size() <= 0)
			return 0.0f;

		auto first = keyFrames.front();
		auto last = keyFrames.back();

		if (keyFrames.size() == 1 || frame <= first.Frame)
			return first.Value;

		if (frame >= last.Frame)
			return last.Value;

		const KeyFrame* start = &keyFrames[0];
		const KeyFrame* end = nullptr;

		for (int i = 1; i < keyFrames.size(); i++)
		{
			end = &keyFrames[i];
			if (end->Frame >= frame)
				break;
			start = end;
		}

		float range = end->Frame - start->Frame;
		float t = (frame - start->Frame) / range;

		return (((((((t * t) * t) - ((t * t) * 2.0)) + t) * start->Interpolation)
			+ ((((t * t) * t) - (t * t)) * end->Interpolation)) * range)
			+ (((((t * t) * 3.0) - (((t * t) * t) * 2.0)) * end->Value)
				+ ((((((t * t) * t) * 2.0) - ((t * t) * 3.0)) + 1.0) * start->Value));
	}

	void AetMgr::Interpolate(const AnimationData& animationData, float frame, Properties* properties)
	{
		float* results = reinterpret_cast<float*>(properties);

		for (std::vector<KeyFrame>* keyFrames = &animationData.Properties->OriginX; keyFrames <= &animationData.Properties->Opacity; keyFrames++)
		{
			*results = AetMgr::Interpolate(*keyFrames, frame);
			results++;
		}

		// std::vector<KeyFrame>* keyFrames = reinterpret_cast<std::vector<KeyFrame>*>(animationData.PerspectiveProperties.get());
		// properties->Origin.x = AetMgr::Interpolate(animationData.Properties->OriginX, frame);
		// properties->Origin.y = AetMgr::Interpolate(animationData.Properties->OriginY, frame);
	}
}
#include "AetMgr.h"

namespace Auth2D
{
	/*static*/ float AetInterpolateAccurate(size_t count, float* keyFrames, float inputFrame)
	{
		float *valueInterpolationPairs = &keyFrames[count];

		if (!count)
			return 0.0;

		if (count == 1)
			return *keyFrames;

		if (inputFrame <= *keyFrames)
			return *valueInterpolationPairs;

		float *keyFrame = keyFrames;
		float *firstValue = &keyFrames[count];

		if (inputFrame >= *(valueInterpolationPairs - 1))
			return valueInterpolationPairs[(2 * count - 2)];

		float *startFrame = &keyFrames[(4 * count) >> 3];

		if (keyFrames < startFrame)
		{
			do
			{
				if (inputFrame >= *startFrame)
					keyFrame = startFrame;
				else
					firstValue = startFrame;
				startFrame = &keyFrame[((char *)firstValue - (char *)keyFrame) >> 3];
			} while (keyFrame < startFrame);
		}

		if (valueInterpolationPairs - 1 <= startFrame)
			return valueInterpolationPairs[(2 * count - 2)];

		size_t pairIndex = (((char *)startFrame - (char *)keyFrames) >> 1) & 0xFFFFFFFFFFFFFFFEui64;

		float range = startFrame[1] - startFrame[0];
		float t = (inputFrame - startFrame[0]) / range;

		float startValue = valueInterpolationPairs[pairIndex + 0];
		float startInterpolation = valueInterpolationPairs[pairIndex + 1];

		float endValue = valueInterpolationPairs[pairIndex + 2];
		float endInterpolation = valueInterpolationPairs[pairIndex + 3];

		return (((((((t * t) * t) - ((t * t) * 2.0)) + t) * startInterpolation)
			+ ((((t * t) * t) - (t * t)) * endInterpolation)) * range)
			+ (((((t * t) * 3.0) - (((t * t) * t) * 2.0)) * endValue)
				+ ((((((t * t) * t) * 2.0) - ((t * t) * 3.0)) + 1.0) * startValue));
	}

	float AetMgr::Interpolate(std::vector<KeyFrame>& keyFrames, float frame)
	{
		if (keyFrames.size() <= 0)
			return 0.0f;

		auto first = keyFrames.front();
		auto last = keyFrames.back();

		if (keyFrames.size() == 1 || frame <= first.Frame)
			return first.Value;

		if (frame >= last.Frame)
			return last.Value;

		KeyFrame* start = &keyFrames[0];
		KeyFrame* end = nullptr;

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
}
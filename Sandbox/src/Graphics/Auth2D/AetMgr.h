#pragma once
#include "FileSystem/Format/AetSet.h"

namespace Auth2D
{
	using namespace FileSystem;

	float AetInterpolateAccurate(size_t count, float* keyFrames, float inputFrame);

	class AetMgr
	{
	public:

		static float Interpolate(std::vector<KeyFrame>& keyFrames, float frame);

	private:
	};
}
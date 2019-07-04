#pragma once
#include "Types.h"
#include "FileSystem/Format/AetSet.h"

namespace Auth2D
{
	using namespace FileSystem;

	struct Properties
	{
		vec2 Origin;
		vec2 Position;
		float Rotation;
		vec2 Scale;
		float Opcaity;
	};

	class AetMgr
	{
	public:

		static float Interpolate(const std::vector<KeyFrame>& keyFrames, float frame);
		static void Interpolate(const AnimationData& animationData, float frame, Properties* properties);

	private:
	};
}
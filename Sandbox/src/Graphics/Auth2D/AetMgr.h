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
		struct ObjCache
		{
			Properties Properties;
			int32_t SpriteIndex;
			AetRegion* Region;
			AetBlendMode BlendMode;
		};

		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetLayer* aetLayer, float frame);
		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetObj* aetObj, float frame);

		static float Interpolate(const std::vector<KeyFrame>& keyFrames, float frame);
		static void Interpolate(const AnimationData& animationData, Properties* properties, float frame);

	private:
		static void InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, Properties* parentProperties, const AetObj* aetObj, float frame);
		static void InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, Properties* parentProperties, const AetObj* aetObj, float frame);
		static void InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, Properties* parentProperties, const AetObj* aetObj, float frame);
	};
}
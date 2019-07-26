#pragma once
#include "Types.h"
#include "FileSystem/Format/AetSet.h"

namespace Auth2D
{
	using namespace FileSystem;

	enum PropertyType_Enum
	{
		PropertyType_OriginX,
		PropertyType_OriginY,
		PropertyType_PositionX,
		PropertyType_PositionY,
		PropertyType_Rotation,
		PropertyType_ScaleX,
		PropertyType_ScaleY,
		PropertyType_Opacity,
		PropertyType_Count,
	};

	struct Properties
	{
		vec2 Origin;
		vec2 Position;
		float Rotation;
		vec2 Scale;
		float Opacity;
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
			const AetObj* AetObj;
		};

		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetLayer* aetLayer, float frame);
		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetObj* aetObj, float frame);

		static float Interpolate(const std::vector<KeyFrame>& keyFrames, float frame);
		static void Interpolate(const AnimationData* animationData, Properties* properties, float frame);

		static KeyFrame* GetKeyFrameAt(KeyFrameCollection& keyFrames, float frame);

	private:
		static void InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame);
		static void InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame);
		static void InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame);
	};
}
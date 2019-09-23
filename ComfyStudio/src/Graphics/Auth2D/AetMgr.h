#pragma once
#include "Types.h"
#include "FileSystem/Format/AetSet.h"

namespace Graphics::Auth2D
{
	using namespace FileSystem;

	struct PropertyTypeFlags
	{
		bool OriginX : 1;
		bool OriginY : 1;
		bool PositionX : 1;
		bool PositionY : 1;
		bool Rotation : 1;
		bool ScaleX : 1;
		bool ScaleY : 1;
		bool Opacity : 1;
	};

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

		bool operator== (const Properties& other) const;
		bool operator!= (const Properties& other) const;
	};

	class AetMgr
	{
	public:
		struct ObjCache
		{
			Properties Properties;
			int32_t SpriteIndex;
			const AetRegion* Region;
			AetBlendMode BlendMode;
			const AetObj* AetObj;
			bool UseTextureMask;
			bool Visible;
		};

		static void GetAddObjects(Vector<AetMgr::ObjCache>& objects, const AetLayer* aetLayer, frame_t frame);
		static void GetAddObjects(Vector<AetMgr::ObjCache>& objects, const AetObj* aetObj, frame_t frame);

		static float Interpolate(const Vector<AetKeyFrame>& keyFrames, frame_t frame);
		static void Interpolate(const AnimationData* animationData, Properties* properties, frame_t frame);

		static AetKeyFrame* GetKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame);

	private:
		static void InternalAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
		static void InternalPicAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
		static void InternalEffAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
	};
}
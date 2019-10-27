#pragma once
#include "Types.h"
#include "AetSet.h"

namespace Graphics
{
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
		// NOTE: Arbitrary safety limit, prevent stack overflows no matter the input
		static constexpr int32_t ParentRecursionLimit = 0x100;

		static constexpr Properties DefaultProperites =
		{
			vec2(0.0f),	// Origin
			vec2(0.0f),	// Position
			0.0f,		// Rotation
			vec2(1.0f),	// Scale
			1.0f,		// Opacity
		};

		struct ObjCache
		{
			Properties Properties;
			int32_t SpriteIndex;
			const AetRegion* Region;
			AetBlendMode BlendMode;
			const AetObj* FirstParent;
			const AetObj* Source;
			bool UseTextureMask;
			bool Visible;
		};

		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetLayer* aetLayer, frame_t frame);
		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetObj* aetObj, frame_t frame);

		static float Interpolate(const AetKeyFrame* start, const AetKeyFrame* end, frame_t frame);
		static float Interpolate(const std::vector<AetKeyFrame>& keyFrames, frame_t frame);
		static void Interpolate(const AnimationData* animationData, Properties* properties, frame_t frame);

		// NOTE: Threshold frame foat comparison
		static bool AreFramesTheSame(frame_t frameA, frame_t frameB);

		static AetKeyFrame* GetKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame);
		
		static void InsertKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame, float value);
		static void DeleteKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame);
		
		// NOTE: Because a KeyFrameCollection is expected to always be sorted
		static void SortKeyFrames(KeyFrameCollection& keyFrames);

		// NOTE: To be used after changing the LoopStart frame of an AetObj
		static void OffsetAllKeyFrames(KeyFrameProperties& properties, frame_t frameIncrement);

		// NOTE: Recursively add the properties of the parent obj to the input properties if there is one
		static void OffsetByParentProperties(Properties& properties, const AetObj* parent, frame_t frame, int32_t& recursionCount);

		// NOTE: To easily navigate between layer references in the tree view
		static void FindAddLayerUsages(const RefPtr<Aet>& aetToSearch, const RefPtr<AetLayer>& layerToFind, std::vector<RefPtr<AetObj>*>& outObjects);

	private:
		static void InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
		static void InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
		static void InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame);
	};
}
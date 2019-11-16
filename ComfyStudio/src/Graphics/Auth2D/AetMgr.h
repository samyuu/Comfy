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

		bool operator==(const Properties& other) const;
		bool operator!=(const Properties& other) const;
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
			const AetSurface* Surface;
			AetBlendMode BlendMode;
			const AetLayer* FirstParent;
			const AetLayer* Source;
			bool UseTextureMask;
			bool Visible;
		};

		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetComposition* comp, frame_t frame);
		static void GetAddObjects(std::vector<AetMgr::ObjCache>& objects, const AetLayer* layer, frame_t frame);

		static float Interpolate(const AetKeyFrame* start, const AetKeyFrame* end, frame_t frame);
		static float Interpolate(const std::vector<AetKeyFrame>& keyFrames, frame_t frame);
		static void Interpolate(const AetAnimationData* animationData, Properties* properties, frame_t frame);

		// NOTE: Threshold frame foat comparison
		static bool AreFramesTheSame(frame_t frameA, frame_t frameB);

		static AetKeyFrame* GetKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame);
		
		static void InsertKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame, float value);
		static void DeleteKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame);
		
		// NOTE: Because a KeyFrameCollection is expected to always be sorted
		static void SortKeyFrames(KeyFrameCollection& keyFrames);

		// NOTE: To be used after changing the StartFrame frame of a layer
		static void OffsetAllKeyFrames(AetKeyFrameProperties& properties, frame_t frameIncrement);

		// NOTE: Recursively add the properties of the parent layer to the input properties if there is one
		static void OffsetByParentProperties(Properties& properties, const AetLayer* parent, frame_t frame, int32_t& recursionCount);

		// NOTE: To easily navigate between composition references in the tree view
		static void FindAddCompositionUsages(const RefPtr<Aet>& aetToSearch, const RefPtr<AetComposition>& compToFind, std::vector<RefPtr<AetLayer>*>& outObjects);

	private:
		static void InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame);
		static void InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame);
		static void InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame);
	};
}
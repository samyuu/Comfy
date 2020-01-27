#pragma once
#include "Types.h"
#include "Transform2D.h"
#include "AetSet.h"

namespace Graphics
{
	class AetMgr
	{
	public:
		// NOTE: Arbitrary safety limit, prevent stack overflows no matter the input
		static constexpr int32_t ParentRecursionLimit = 0x100;

		struct ObjCache
		{
			Transform2D Transform;
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
		
		static float GetValueAt(const std::vector<AetKeyFrame>& keyFrames, frame_t frame);
		static float GetValueAt(const AetProperty1D& property, frame_t frame);
		static vec2 GetValueAt(const AetProperty2D& property, frame_t frame);
		
		static Transform2D GetTransformAt(const AetTransform& transform, frame_t frame);
		static Transform2D GetTransformAt(const AetAnimationData& animationData, frame_t frame);

		// NOTE: Threshold frame foat comparison
		static bool AreFramesTheSame(frame_t frameA, frame_t frameB);

		static AetKeyFrame* GetKeyFrameAt(AetProperty1D& property, frame_t frame);
		
		static void InsertKeyFrameAt(std::vector<AetKeyFrame>& keyFrames, frame_t frame, float value);
		static void DeleteKeyFrameAt(std::vector<AetKeyFrame>& keyFrames, frame_t frame);
		
		// NOTE: Because a KeyFrameCollection is expected to always be sorted
		static void SortKeyFrames(std::vector<AetKeyFrame>& keyFrames);

		// NOTE: To be used after changing the StartFrame frame of a layer
		static void OffsetAllKeyFrames(AetTransform& transform, frame_t frameIncrement);

		// NOTE: Recursively add the properties of the parent layer to the input properties if there is one
		static void ApplyParentTransform(Transform2D& outTransform, const AetLayer* parent, frame_t frame, int32_t& recursionCount);

		// NOTE: To easily navigate between composition references in the tree view
		static void FindAddCompositionUsages(const RefPtr<Aet>& aetToSearch, const RefPtr<AetComposition>& compToFind, std::vector<RefPtr<AetLayer>*>& outObjects);

	private:
		static void InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame);
		static void InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame);
		static void InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame);
	};
}
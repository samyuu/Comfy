#pragma once
#include "Types.h"
#include "AetSet.h"
#include "../Transform2D.h"

namespace Comfy::Graphics::Aet
{
	namespace Util
	{
		// NOTE: Arbitrary safety limit to prevent stack overflows no matter the input
		constexpr i32 ParentRecursionLimit = 64;

		// NOTE: Represents a fully processed layer item
		struct Obj
		{
			Transform2D Transform;
			// TODO: mat4 TransformMatrix;
			const Video* Video;
			const Layer* FirstParent;
			const Layer* SourceLayer;
			i32 SpriteFrame;
			AetBlendMode BlendMode;
			bool UseTrackMatte;
			bool IsVisible;
		};

		using ObjCache = std::vector<Obj>;

		void GetAddObjectsAt(ObjCache& outObjs, const Composition& comp, frame_t frame);
		void GetAddObjectsAt(ObjCache& outObjs, const Layer& layer, frame_t frame);

		float Interpolate(const KeyFrame& start, const KeyFrame& end, frame_t frame);

		float GetValueAt(const std::vector<KeyFrame>& keyFrames, frame_t frame);
		float GetValueAt(const Property1D& property, frame_t frame);
		vec2 GetValueAt(const Property2D& property, frame_t frame);

		Transform2D GetTransformAt(const LayerVideo2D& LayerVideo2D, frame_t frame);
		Transform2D GetTransformAt(const LayerVideo& layerVideo, frame_t frame);

		// NOTE: Threshold frame float comparison
		bool AreFramesTheSame(frame_t frameA, frame_t frameB);

		KeyFrame* GetKeyFrameAt(Property1D& property, frame_t frame);

		void InsertKeyFrameAt(std::vector<KeyFrame>& keyFrames, frame_t frame, float value);
		void DeleteKeyFrameAt(std::vector<KeyFrame>& keyFrames, frame_t frame);

		// NOTE: Because keyframes are always expected to be sorted
		void SortKeyFrames(std::vector<KeyFrame>& keyFrames);

		// NOTE: To be used after changing the start frame of a layer
		void OffsetAllKeyFrames(LayerVideo2D& layerVideo2D, frame_t frameIncrement);

		// NOTE: Recursively add the transform of the parent layer to the input transform if there is one
		void ApplyParentTransform(Transform2D& outTransform, const Layer* parent, frame_t frame, i32& recursionCount);

		// NOTE: To easily navigate between composition references in the aet editor tree view
		template <typename Func>
		void FindCompositionLayerUsages(const Scene& sceneToSearch, const Composition& compToFind, Func func)
		{
			sceneToSearch.ForEachComp([&](const auto& comp)
			{
				for (auto& layer : comp->GetLayers())
				{
					if (layer->GetCompItem().get() == &compToFind)
						func(layer);
				}
			});
		}

		namespace Detail
		{
			void AddLayerItemObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame);
			void AddVideoObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame);
			void AddCompObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame);
		}
	};
}

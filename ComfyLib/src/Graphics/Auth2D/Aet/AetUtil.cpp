#include "AetUtil.h"

namespace Comfy::Graphics::Aet
{
	namespace Util
	{
		void CombineTransforms(const Transform2D& input, Transform2D & inOutput)
		{
			// BUG: This is still not 100% accurate (?)
			inOutput.Position -= input.Origin;
			inOutput.Position *= input.Scale;

			if (input.Rotation != 0.0f)
			{
				const float radians = glm::radians(input.Rotation);
				const float sin = glm::sin(radians);
				const float cos = glm::cos(radians);

				inOutput.Position = vec2(inOutput.Position.x * cos - inOutput.Position.y * sin, inOutput.Position.x * sin + inOutput.Position.y * cos);
			}

			inOutput.Position += input.Position;

			if ((input.Scale.x < 0.0f) ^ (input.Scale.y < 0.0f))
				inOutput.Rotation *= -1.0f;

			inOutput.Rotation += input.Rotation;

			inOutput.Scale *= input.Scale;
			inOutput.Opacity *= input.Opacity;
		}

		Transform2D CombineTransformsCopy(const Transform2D& source, const Transform2D& toApply)
		{
			auto copy = source;
			CombineTransforms(toApply, copy);
			return copy;
		}

		void GetAddObjectsAt(ObjCache& outObjs, const Composition& comp, frame_t frame)
		{
			std::for_each(comp.GetLayers().rbegin(), comp.GetLayers().rend(), [&](const auto& it)
			{
				GetAddObjectsAt(outObjs, *it, frame);
			});
		}

		void GetAddObjectsAt(ObjCache& outObjs, const Layer& layer, frame_t frame)
		{
			const auto transform = Transform2D(vec2(0.0f));
			Detail::AddLayerItemObjects(outObjs, transform, layer, frame);

			if (layer.ItemType == ItemType::Composition)
			{
				for (auto& object : outObjs)
				{
					if (object.FirstParent == nullptr)
						object.FirstParent = &layer;
				}
			}
		}

		float Interpolate(const KeyFrame& start, const KeyFrame& end, frame_t frame)
		{
			const float range = end.Frame - start.Frame;
			const float t = (frame - start.Frame) / range;

			return (((((((t * t) * t) - ((t * t) * 2.0f)) + t) * start.Curve)
				+ ((((t * t) * t) - (t * t)) * end.Curve)) * range)
				+ (((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end.Value)
					+ ((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start.Value));
		}

		float GetValueAt(const std::vector<KeyFrame>& keyFrames, frame_t frame)
		{
			if (keyFrames.size() <= 0)
				return 0.0f;

			const auto& first = keyFrames.front();
			const auto& last = keyFrames.back();

			if (keyFrames.size() == 1 || frame <= first.Frame)
				return first.Value;

			if (frame >= last.Frame)
				return last.Value;

			const KeyFrame* start = &first;
			const KeyFrame* end = start;

			for (int i = 1; i < keyFrames.size(); i++)
			{
				end = &keyFrames[i];
				if (end->Frame >= frame)
					break;
				start = end;
			}

			return Interpolate(*start, *end, frame);
		}

		float GetValueAt(const Property1D& property, frame_t frame)
		{
			return GetValueAt(property.Keys, frame);
		}

		vec2 GetValueAt(const Property2D& property, frame_t frame)
		{
			return vec2(GetValueAt(property.X, frame), GetValueAt(property.Y, frame));
		}

		Transform2D GetTransformAt(const LayerVideo2D& layerVideo2D, frame_t frame)
		{
			Transform2D result;
			result.Origin = GetValueAt(layerVideo2D.Origin, frame);
			result.Position = GetValueAt(layerVideo2D.Position, frame);
			result.Rotation = GetValueAt(layerVideo2D.Rotation, frame);
			result.Scale = GetValueAt(layerVideo2D.Scale, frame);
			result.Opacity = GetValueAt(layerVideo2D.Opacity, frame);
			return result;
		}

		Transform2D GetTransformAt(const LayerVideo& layerVideo, frame_t frame)
		{
			return GetTransformAt(layerVideo.Transform, frame);
		}

		Transform2D GetPositionTransformAt(const LayerVideo2D& LayerVideo2D, frame_t frame)
		{
			Transform2D result;
			result.Origin = vec2(0.0f);
			result.Position = GetValueAt(LayerVideo2D.Position, frame);
			result.Rotation = 0.0f;
			result.Scale = vec2(1.0f);
			result.Opacity = GetValueAt(LayerVideo2D.Opacity, frame);
			return result;
		}

		Transform2D GetPositionTransformAt(const LayerVideo& layerVideo, frame_t frame)
		{
			return GetPositionTransformAt(layerVideo.Transform, frame);
		}

		bool AreFramesTheSame(frame_t frameA, frame_t frameB)
		{
			// NOTE: Completely arbitrary threshold, in practice even a frame round or an int cast would probably suffice
			constexpr frame_t frameComparisonThreshold = 0.001f;

			return std::abs(frameA - frameB) < frameComparisonThreshold;
		}

		KeyFrame* GetKeyFrameAt(Property1D& property, frame_t frame)
		{
			// NOTE: The aet editor should always try to prevent this itself
			assert(!property->empty());

			// TODO: This could implement a binary search although the usually small number keyframes might not warrant it
			for (auto& keyFrame : property.Keys)
			{
				if (AreFramesTheSame(keyFrame.Frame, frame))
					return &keyFrame;
			}

			return nullptr;
		}

		void InsertKeyFrameAt(std::vector<KeyFrame>& keyFrames, frame_t frame, float value)
		{
			keyFrames.emplace_back(frame, value);
			SortKeyFrames(keyFrames);
		}

		void DeleteKeyFrameAt(std::vector<KeyFrame>& keyFrames, frame_t frame)
		{
			auto existing = std::find_if(keyFrames.begin(), keyFrames.end(), [frame](const KeyFrame& keyFrame)
			{
				return AreFramesTheSame(keyFrame.Frame, frame);
			});

			if (existing != keyFrames.end())
			{
				keyFrames.erase(existing);
			}
			else
			{
				assert(false);
			}
		}

		void SortKeyFrames(std::vector<KeyFrame>& keyFrames)
		{
			std::sort(keyFrames.begin(), keyFrames.end(), [](const KeyFrame& keyFrameA, const KeyFrame& keyFrameB)
			{
				return keyFrameA.Frame < keyFrameB.Frame;
			});
		}

		void OffsetAllKeyFrames(LayerVideo2D& layerVideo2D, frame_t frameIncrement)
		{
			for (Transform2DField i = 0; i < Transform2DField_Count; i++)
			{
				for (auto& keyFrame : layerVideo2D[i].Keys)
					keyFrame.Frame += frameIncrement;
			}
		}

		void ApplyParentTransform(Transform2D& outTransform, const Layer* parent, frame_t frame, i32& recursionCount)
		{
			assert(recursionCount < ParentRecursionLimit);
			if (parent == nullptr || recursionCount > ParentRecursionLimit)
				return;

			recursionCount++;

			const Layer* parentParent = parent->GetRefParentLayer();
			assert(parentParent != parent);

			const Transform2D parentTransform = GetTransformAt(*parent->LayerVideo, frame);
			outTransform.Position += parentTransform.Position - parentTransform.Origin;
			outTransform.Rotation += parentTransform.Rotation;
			outTransform.Scale *= parentTransform.Scale;

			if (parentParent != nullptr && parentParent != parent)
				ApplyParentTransform(outTransform, parentParent, frame, recursionCount);
		}

		namespace Detail
		{
			void AddLayerItemObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame)
			{
				if (layer.ItemType == ItemType::Video)
					AddVideoObjects(outObjs, parentTransform, layer, frame);
				else if (layer.ItemType == ItemType::Composition)
					AddCompObjects(outObjs, parentTransform, layer, frame);
			}

			void AddVideoObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame)
			{
				if (frame < layer.StartFrame || frame >= layer.EndFrame)
					return;

				Obj& objCache = outObjs.emplace_back();
				objCache.FirstParent = nullptr;
				objCache.SourceLayer = &layer;
				objCache.Video = layer.GetVideoItem();
				objCache.BlendMode = layer.LayerVideo->TransferMode.BlendMode;
				objCache.UseTrackMatte = layer.LayerVideo->TransferMode.TrackMatte == TrackMatte::Alpha;
				objCache.IsVisible = layer.GetIsVisible();

				if (objCache.Video != nullptr && objCache.Video->Sources.size() > 1)
				{
					objCache.SpriteFrame = static_cast<i32>(glm::round((frame + layer.StartOffset) * layer.TimeScale * objCache.Video->FilesPerFrame));
					objCache.SpriteFrame = glm::clamp(objCache.SpriteFrame, 0, static_cast<int>(objCache.Video->Sources.size()) - 1);
				}
				else
				{
					objCache.SpriteFrame = 0;
				}

				objCache.Transform = GetTransformAt(*layer.LayerVideo, frame);

				i32 recursionCount = 0;
				ApplyParentTransform(objCache.Transform, layer.GetRefParentLayer(), frame, recursionCount);
				CombineTransforms(parentTransform, objCache.Transform);
			}

			void AddCompObjects(ObjCache& outObjs, const Transform2D& parentTransform, const Layer& layer, frame_t frame)
			{
				if (frame < layer.StartFrame || frame >= layer.EndFrame || !layer.GetIsVisible())
					return;

				const Composition* comp = layer.GetCompItem();
				if (comp == nullptr)
					return;

				Transform2D transform = GetTransformAt(*layer.LayerVideo, frame);

				i32 recursionCount = 0;
				ApplyParentTransform(transform, layer.GetRefParentLayer(), frame, recursionCount);
				CombineTransforms(parentTransform, transform);

				const frame_t adjustedFrame = ((frame - layer.StartFrame) * layer.TimeScale) + layer.StartOffset;
				std::for_each(comp->GetLayers().rbegin(), comp->GetLayers().rend(), [&](const auto& it)
				{
					AddLayerItemObjects(outObjs, transform, *it, adjustedFrame);
				});
			}
		}
	}
}

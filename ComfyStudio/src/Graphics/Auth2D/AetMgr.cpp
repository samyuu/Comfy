#include "AetMgr.h"

namespace Graphics
{
	static_assert((sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection)) == (sizeof(Transform2D) / sizeof(float)),
		"The Transform2D struct must have an equal number of float fields as the KeyFrameCollectionArray has KeyFrameCollections");

	static_assert(sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection) == Transform2D_Count);

	namespace
	{
		void TransformByParent(const Transform2D& input, Transform2D& output)
		{
			// BUG: This is still not 100% accurate (?)

			output.Position -= input.Origin;
			output.Position *= input.Scale;

			if (input.Rotation != 0.0f)
			{
				float radians = glm::radians(input.Rotation);
				float sin = glm::sin(radians);
				float cos = glm::cos(radians);

				output.Position = vec2(output.Position.x * cos - output.Position.y * sin, output.Position.x * sin + output.Position.y * cos);
			}

			output.Position += input.Position;

			if ((input.Scale.x < 0.0f) ^ (input.Scale.y < 0.0f))
				output.Rotation *= -1.0f;

			output.Rotation += input.Rotation;

			output.Scale *= input.Scale;
			output.Opacity *= input.Opacity;
		}
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetComposition* comp, frame_t frame)
	{
		for (int i = static_cast<int>(comp->size()) - 1; i >= 0; i--)
			GetAddObjects(objects, comp->GetLayerAt(i), frame);
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetLayer* layer, frame_t frame)
	{
		Transform2D transform = Transform2D(vec2(0.0f));
		InternalAddObjects(objects, &transform, layer, frame);

		if (layer->Type == AetLayerType::Eff)
		{
			for (auto& object : objects)
			{
				if (object.FirstParent == nullptr)
					object.FirstParent = layer;
			}
		}
	}

	float AetMgr::Interpolate(const AetKeyFrame* start, const AetKeyFrame* end, frame_t frame)
	{
		float range = end->Frame - start->Frame;
		float t = (frame - start->Frame) / range;

		return (((((((t * t) * t) - ((t * t) * 2.0f)) + t) * start->Curve)
			+ ((((t * t) * t) - (t * t)) * end->Curve)) * range)
			+ (((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end->Value)
				+ ((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start->Value));
	}

	float AetMgr::GetValueAt(const std::vector<AetKeyFrame>& keyFrames, frame_t frame)
	{
		if (keyFrames.size() <= 0)
			return 0.0f;

		auto first = keyFrames.front();
		auto last = keyFrames.back();

		if (keyFrames.size() == 1 || frame <= first.Frame)
			return first.Value;

		if (frame >= last.Frame)
			return last.Value;

		const AetKeyFrame* start = &keyFrames[0];
		const AetKeyFrame* end = start;

		for (int i = 1; i < keyFrames.size(); i++)
		{
			end = &keyFrames[i];
			if (end->Frame >= frame)
				break;
			start = end;
		}

		return Interpolate(start, end, frame);
	}

	vec2 AetMgr::GetValueAt(const std::vector<AetKeyFrame>& keyFramesX, const std::vector<AetKeyFrame>& keyFramesY, frame_t frame)
	{
		return vec2(AetMgr::GetValueAt(keyFramesX, frame), AetMgr::GetValueAt(keyFramesY, frame));
	}

	Transform2D AetMgr::GetTransformAt(const AetKeyFrameProperties& properties, frame_t frame)
	{
		Transform2D result;
		result.Origin = AetMgr::GetValueAt(properties.OriginX(), properties.OriginY(), frame);
		result.Position = AetMgr::GetValueAt(properties.PositionX(), properties.PositionY(), frame);
		result.Rotation = AetMgr::GetValueAt(properties.Rotation(), frame);
		result.Scale = AetMgr::GetValueAt(properties.ScaleX(), properties.ScaleY(), frame);
		result.Opacity = AetMgr::GetValueAt(properties.Opacity(), frame);
		return result;
	}

	Transform2D AetMgr::GetTransformAt(const AetAnimationData& animationData, frame_t frame)
	{
		return AetMgr::GetTransformAt(animationData.Properties, frame);
	}

	bool AetMgr::AreFramesTheSame(frame_t frameA, frame_t frameB)
	{
		// NOTE: Completely arbitrary threshold, in practice even a frame round or an int cast would probably suffice
		constexpr frame_t frameComparisonThreshold = 0.001f;
		
		return std::abs(frameA - frameB) < frameComparisonThreshold;
	}

	AetKeyFrame* AetMgr::GetKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame)
	{
		// NOTE: The aet editor should always try to prevent this itself
		assert(keyFrames.size() > 0);

		// TODO: This could implement a binary search although the usually small number keyframes might not warrant it
		for (auto& keyFrame : keyFrames)
		{
			if (AreFramesTheSame(keyFrame.Frame, frame))
				return &keyFrame;
		}

		return nullptr;
	}

	void AetMgr::InsertKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame, float value)
	{
		keyFrames.emplace_back(frame, value);
		AetMgr::SortKeyFrames(keyFrames);
	}

	void AetMgr::DeleteKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame)
	{
		auto existing = std::find_if(keyFrames.begin(), keyFrames.end(), [frame](const AetKeyFrame& keyFrame)
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

	void AetMgr::SortKeyFrames(KeyFrameCollection& keyFrames)
	{
		std::sort(keyFrames.begin(), keyFrames.end(), [](const AetKeyFrame& keyFrameA, const AetKeyFrame& keyFrameB)
		{
			return keyFrameA.Frame < keyFrameB.Frame;
		});
	}

	void AetMgr::OffsetAllKeyFrames(AetKeyFrameProperties& properties, frame_t frameIncrement)
	{
		for (auto& keyFrames : properties)
		{
			for (auto& keyFrame : keyFrames)
				keyFrame.Frame += frameIncrement;
		}
	}

	void AetMgr::ApplyParentTransform(Transform2D& outTransform, const AetLayer* parent, frame_t frame, int32_t& recursionCount)
	{
		assert(recursionCount < ParentRecursionLimit);
		if (parent == nullptr || recursionCount > ParentRecursionLimit)
			return;

		recursionCount++;

		const AetLayer* parentParent = parent->GetReferencedParentLayer();
		assert(parentParent != parent);

		const Transform2D parentTransform = AetMgr::GetTransformAt(*parent->AnimationData, frame);

		outTransform.Position += parentTransform.Position - parentTransform.Origin;
		outTransform.Rotation += parentTransform.Rotation;
		outTransform.Scale *= parentTransform.Scale;

		if (parentParent != nullptr && parentParent != parent)
			ApplyParentTransform(outTransform, parentParent, frame, recursionCount);
	}

	void AetMgr::FindAddCompositionUsages(const RefPtr<Aet>& aetToSearch, const RefPtr<AetComposition>& compToFind, std::vector<RefPtr<AetLayer>*>& outObjects)
	{
		const auto compSearchFunction = [&compToFind, &outObjects](const RefPtr<AetComposition>& compToSearch)
		{
			for (RefPtr<AetLayer>& layer : *compToSearch)
			{
				if (layer->GetReferencedComposition() == compToFind)
					outObjects.push_back(&layer);
			}
		};

		compSearchFunction(aetToSearch->RootComposition);

		for (auto& compToTest : aetToSearch->Compositions)
			compSearchFunction(compToTest);
	}

	void AetMgr::InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame)
	{
		if (layer->Type == AetLayerType::Pic)
		{
			InternalPicAddObjects(objects, parentTransform, layer, frame);
		}
		else if (layer->Type == AetLayerType::Eff)
		{
			InternalEffAddObjects(objects, parentTransform, layer, frame);
		}
	}

	void AetMgr::InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame)
	{
		assert(layer->Type == AetLayerType::Pic);

		if (frame < layer->StartFrame || frame >= layer->EndFrame)
			return;

		objects.emplace_back();
		ObjCache& objCache = objects.back();

		objCache.FirstParent = nullptr;
		objCache.Source = layer;
		objCache.Surface = layer->GetReferencedSurface();
		objCache.BlendMode = layer->AnimationData->BlendMode;
		objCache.UseTextureMask = layer->AnimationData->UseTextureMask;
		objCache.Visible = layer->Flags.Visible;

		if (objCache.Surface != nullptr && objCache.Surface->SpriteCount() > 1)
		{
			// BUG: This should factor in the layer->StartFrame (?)
			objCache.SpriteIndex = static_cast<int>(glm::round((frame + layer->StartOffset) * layer->PlaybackSpeed));
			objCache.SpriteIndex = glm::clamp(objCache.SpriteIndex, 0, static_cast<int>(objCache.Surface->SpriteCount()) - 1);
		}
		else
		{
			objCache.SpriteIndex = 0;
		}

		objCache.Transform = AetMgr::GetTransformAt(*layer->AnimationData, frame);

		int32_t recursionCount = 0;
		AetMgr::ApplyParentTransform(objCache.Transform, layer->GetReferencedParentLayer(), frame, recursionCount);
		TransformByParent(*parentTransform, objCache.Transform);
	}

	void AetMgr::InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Transform2D* parentTransform, const AetLayer* layer, frame_t frame)
	{
		assert(layer->Type == AetLayerType::Eff);

		if (frame < layer->StartFrame || frame >= layer->EndFrame || !layer->Flags.Visible)
			return;

		const AetComposition* comp = layer->GetReferencedComposition();
		if (comp == nullptr)
			return;

		Transform2D effTransform = AetMgr::GetTransformAt(*layer->AnimationData, frame);

		int32_t recursionCount = 0;
		AetMgr::ApplyParentTransform(effTransform, layer->GetReferencedParentLayer(), frame, recursionCount);
		TransformByParent(*parentTransform, effTransform);

		frame_t adjustedFrame = ((frame - layer->StartFrame) * layer->PlaybackSpeed) + layer->StartOffset;

		for (int i = static_cast<int>(comp->size()) - 1; i >= 0; i--)
			InternalAddObjects(objects, &effTransform, comp->GetLayerAt(i), adjustedFrame);
	}
}
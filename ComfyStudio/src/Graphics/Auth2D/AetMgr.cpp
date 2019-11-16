#include "AetMgr.h"

namespace Graphics
{
	static_assert((sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection)) == (sizeof(Properties) / sizeof(float)),
		"The AetMgr Properties struct must have an equal number of float fields as the KeyFrameCollectionArray has KeyFrameCollections");

	static_assert(sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection) == PropertyType_Count);

	bool Properties::operator==(const Properties& other) const
	{
		return (Origin == other.Origin) && (Position == other.Position) && (Rotation == other.Rotation) && (Scale == other.Scale) && (Opacity == other.Opacity);
	}

	bool Properties::operator!=(const Properties& other) const
	{
		return !(*this == other);
	}

	static void TransformProperties(const Properties& input, Properties& output)
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

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetComposition* comp, frame_t frame)
	{
		for (int i = static_cast<int>(comp->size()) - 1; i >= 0; i--)
			GetAddObjects(objects, comp->GetLayerAt(i), frame);
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetLayer* layer, frame_t frame)
	{
		Properties propreties = DefaultProperites;
		InternalAddObjects(objects, &propreties, layer, frame);

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

	float AetMgr::Interpolate(const std::vector<AetKeyFrame>& keyFrames, frame_t frame)
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

	void AetMgr::Interpolate(const AetAnimationData* animationData, Properties* properties, frame_t frame)
	{
		float* results = reinterpret_cast<float*>(properties);

		for (auto& keyFrames : animationData->Properties)
		{
			*results = AetMgr::Interpolate(keyFrames, frame);
			results++;
		}
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

	void AetMgr::OffsetByParentProperties(Properties& properties, const AetLayer* parent, frame_t frame, int32_t& recursionCount)
	{
		assert(recursionCount < ParentRecursionLimit);
		if (parent == nullptr || recursionCount > ParentRecursionLimit)
			return;

		recursionCount++;

		const AetLayer* parentParent = parent->GetReferencedParentLayer();
		assert(parentParent != parent);

		Properties parentProperties;
		Interpolate(parent->AnimationData.get(), &parentProperties, frame);

		properties.Position += parentProperties.Position - parentProperties.Origin;
		properties.Rotation += parentProperties.Rotation;
		properties.Scale *= parentProperties.Scale;

		if (parentParent != nullptr && parentParent != parent)
			OffsetByParentProperties(properties, parentParent, frame, recursionCount);
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

	void AetMgr::InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame)
	{
		if (layer->Type == AetLayerType::Pic)
		{
			InternalPicAddObjects(objects, parentProperties, layer, frame);
		}
		else if (layer->Type == AetLayerType::Eff)
		{
			InternalEffAddObjects(objects, parentProperties, layer, frame);
		}
	}

	void AetMgr::InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame)
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

		Interpolate(layer->AnimationData.get(), &objCache.Properties, frame);

		int32_t recursionCount = 0;
		OffsetByParentProperties(objCache.Properties, layer->GetReferencedParentLayer(), frame, recursionCount);
		TransformProperties(*parentProperties, objCache.Properties);
	}

	void AetMgr::InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetLayer* layer, frame_t frame)
	{
		assert(layer->Type == AetLayerType::Eff);

		if (frame < layer->StartFrame || frame >= layer->EndFrame || !layer->Flags.Visible)
			return;

		const AetComposition* comp = layer->GetReferencedComposition();
		if (comp == nullptr)
			return;

		Properties effProperties;
		Interpolate(layer->AnimationData.get(), &effProperties, frame);

		int32_t recursionCount = 0;
		OffsetByParentProperties(effProperties, layer->GetReferencedParentLayer(), frame, recursionCount);
		TransformProperties(*parentProperties, effProperties);

		frame_t adjustedFrame = ((frame - layer->StartFrame) * layer->PlaybackSpeed) + layer->StartOffset;

		for (int i = static_cast<int>(comp->size()) - 1; i >= 0; i--)
			InternalAddObjects(objects, &effProperties, comp->GetLayerAt(i), adjustedFrame);
	}
}
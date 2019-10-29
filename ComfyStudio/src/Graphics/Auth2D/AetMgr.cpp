#include "AetMgr.h"

namespace Graphics
{
	static_assert((sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection)) == (sizeof(Properties) / sizeof(float)),
		"The AetMgr Properties struct must have an equal number of float fields as the KeyFrameCollectionArray has KeyFrameCollections");

	static_assert(sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection) == PropertyType_Count);

	bool Properties::operator== (const Properties& other) const
	{
		return (Origin == other.Origin) && (Position == other.Position) && (Rotation == other.Rotation) && (Scale == other.Scale) && (Opacity == other.Opacity);
	}

	bool Properties::operator!= (const Properties& other) const
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

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetLayer* aetLayer, frame_t frame)
	{
		for (int i = static_cast<int>(aetLayer->size()) - 1; i >= 0; i--)
			GetAddObjects(objects, aetLayer->GetObjAt(i), frame);
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetObj* aetObj, frame_t frame)
	{
		Properties propreties = DefaultProperites;
		InternalAddObjects(objects, &propreties, aetObj, frame);

		if (aetObj->Type == AetObjType::Eff)
		{
			for (auto& object : objects)
			{
				if (object.FirstParent == nullptr)
					object.FirstParent = aetObj;
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

	void AetMgr::Interpolate(const AnimationData* animationData, Properties* properties, frame_t frame)
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

	void AetMgr::OffsetAllKeyFrames(KeyFrameProperties& properties, frame_t frameIncrement)
	{
		for (auto& keyFrames : properties)
		{
			for (auto& keyFrame : keyFrames)
				keyFrame.Frame += frameIncrement;
		}
	}

	void AetMgr::OffsetByParentProperties(Properties& properties, const AetObj* parent, frame_t frame, int32_t& recursionCount)
	{
		assert(recursionCount < ParentRecursionLimit);
		if (parent == nullptr || recursionCount > ParentRecursionLimit)
			return;

		recursionCount++;

		const AetObj* parentParent = parent->GetReferencedParentObj();
		assert(parentParent != parent);

		Properties parentProperties;
		Interpolate(parent->AnimationData.get(), &parentProperties, frame);

		properties.Position += parentProperties.Position - parentProperties.Origin;
		properties.Rotation += parentProperties.Rotation;
		properties.Scale *= parentProperties.Scale;

		if (parentParent != nullptr && parentParent != parent)
			OffsetByParentProperties(properties, parentParent, frame, recursionCount);
	}

	void AetMgr::FindAddLayerUsages(const RefPtr<Aet>& aetToSearch, const RefPtr<AetLayer>& layerToFind, std::vector<RefPtr<AetObj>*>& outObjects)
	{
		const auto layerSearchFunction = [&layerToFind, &outObjects](const RefPtr<AetLayer>& layerToSearch)
		{
			for (RefPtr<AetObj>& object : *layerToSearch)
			{
				if (object->GetReferencedLayer() == layerToFind)
					outObjects.push_back(&object);
			}
		};

		layerSearchFunction(aetToSearch->RootLayer);

		for (auto& layerToTest : aetToSearch->Layers)
			layerSearchFunction(layerToTest);
	}

	void AetMgr::InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
	{
		if (aetObj->Type == AetObjType::Pic)
		{
			InternalPicAddObjects(objects, parentProperties, aetObj, frame);
		}
		else if (aetObj->Type == AetObjType::Eff)
		{
			InternalEffAddObjects(objects, parentProperties, aetObj, frame);
		}
	}

	void AetMgr::InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
	{
		assert(aetObj->Type == AetObjType::Pic);

		if (frame < aetObj->StartFrame || frame >= aetObj->EndFrame)
			return;

		objects.emplace_back();
		ObjCache& objCache = objects.back();

		objCache.FirstParent = nullptr;
		objCache.Source = aetObj;
		objCache.Region = aetObj->GetReferencedRegion();
		objCache.BlendMode = aetObj->AnimationData->BlendMode;
		objCache.UseTextureMask = aetObj->AnimationData->UseTextureMask;
		objCache.Visible = aetObj->Flags.Visible;

		if (objCache.Region != nullptr && objCache.Region->SpriteCount() > 0)
		{
			// BUG: This should factor in the aetObj->StartFrame (?)
			// NOTE: Is it correct to modulo the index here? Seems to make more sense than just clamping
			objCache.SpriteIndex = static_cast<int>(glm::round(frame)) % objCache.Region->SpriteCount();
		}
		Interpolate(aetObj->AnimationData.get(), &objCache.Properties, frame);

		int32_t recursionCount = 0;
		OffsetByParentProperties(objCache.Properties, aetObj->GetReferencedParentObj(), frame, recursionCount);
		TransformProperties(*parentProperties, objCache.Properties);
	}

	void AetMgr::InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
	{
		assert(aetObj->Type == AetObjType::Eff);

		if (frame < aetObj->StartFrame || frame >= aetObj->EndFrame || !aetObj->Flags.Visible)
			return;

		const AetLayer* aetLayer = aetObj->GetReferencedLayer();
		if (aetLayer == nullptr)
			return;

		Properties effProperties;
		Interpolate(aetObj->AnimationData.get(), &effProperties, frame);

		int32_t recursionCount = 0;
		OffsetByParentProperties(effProperties, aetObj->GetReferencedParentObj(), frame, recursionCount);
		TransformProperties(*parentProperties, effProperties);

		frame_t adjustedFrame = ((frame - aetObj->StartFrame) * aetObj->PlaybackSpeed) + aetObj->StartOffset;

		for (int i = static_cast<int>(aetLayer->size()) - 1; i >= 0; i--)
			InternalAddObjects(objects, &effProperties, aetLayer->GetObjAt(i), adjustedFrame);
	}
}
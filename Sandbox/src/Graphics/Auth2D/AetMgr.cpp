#include "AetMgr.h"

namespace Auth2D
{
	static_assert((sizeof(KeyFrameCollectionArray) / sizeof(KeyFrameCollection)) == (sizeof(Properties) / sizeof(float)),
		"The AetMgr Properties struct must have an equal number of float fields as the KeyFrameCollectionArray has KeyFrameCollections");

	static Properties DefaultProperites =
	{
		vec2(0.0f),	// Origin
		vec2(0.0f),	// Position
		0.0f,		// Rotation
		vec2(1.0f),	// Scale
		1.0f,		// Opacity
	};

	static void TransformProperties(const Properties& input, Properties& output)
	{
		// TODO:
		output.Position -= input.Origin;
		output.Position *= input.Scale;
		if (input.Rotation != 0.0f)
		{
			float radians = glm::radians(input.Rotation);
			float sin = glm::sin(radians);
			float cos = glm::cos(radians);

			output.Position.x = output.Position.x * cos - output.Position.y * sin;
			output.Position.y = output.Position.x * sin + output.Position.y * cos;
		}
		output.Position += input.Position;

		output.Rotation += input.Rotation;
		output.Scale *= input.Scale;
		output.Opacity *= input.Opacity;
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetLayer* aetLayer, float frame)
	{
		for (int i = static_cast<int>(aetLayer->size() - 1); i >= 0; i--)
			GetAddObjects(objects, &aetLayer->at(i), frame);
	}

	void AetMgr::GetAddObjects(std::vector<ObjCache>& objects, const AetObj* aetObj, float frame)
	{
		Properties propreties = DefaultProperites;
		InternalAddObjects(objects, &propreties, aetObj, frame);
	}

	float AetMgr::Interpolate(const std::vector<KeyFrame>& keyFrames, float frame)
	{
		if (keyFrames.size() <= 0)
			return 0.0f;

		auto first = keyFrames.front();
		auto last = keyFrames.back();

		if (keyFrames.size() == 1 || frame <= first.Frame)
			return first.Value;

		if (frame >= last.Frame)
			return last.Value;

		const KeyFrame* start = &keyFrames[0];
		const KeyFrame* end = nullptr;

		for (int i = 1; i < keyFrames.size(); i++)
		{
			end = &keyFrames[i];
			if (end->Frame >= frame)
				break;
			start = end;
		}

		float range = end->Frame - start->Frame;
		float t = (frame - start->Frame) / range;

		return (((((((t * t) * t) - ((t * t) * 2.0f)) + t) * start->Interpolation)
			+ ((((t * t) * t) - (t * t)) * end->Interpolation)) * range)
			+ (((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end->Value)
				+ ((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start->Value));
	}

	void AetMgr::Interpolate(const AnimationData& animationData, Properties* properties, float frame)
	{
		float* results = reinterpret_cast<float*>(properties);

		for (auto& keyFrames : *animationData.Properties.get())
		{
			*results = AetMgr::Interpolate(keyFrames, frame);
			results++;
		}
	}

	void AetMgr::InternalAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame)
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

	void AetMgr::InternalPicAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame)
	{
		assert(aetObj->Type == AetObjType::Pic);

		if (frame < aetObj->LoopStart || frame >= aetObj->LoopEnd || !(aetObj->Flags & AetObjFlags_Visible)) // (aetObj->Flags & AetObjFlags_Visible || aetObj->AnimationData.UseTextureMask) )
			return;

		float adjustedFrame = ((frame - aetObj->LoopStart) * aetObj->PlaybackSpeed) + aetObj->StartFrame;

		objects.emplace_back();
		ObjCache& objCache = objects.back();

		objCache.AetObj = aetObj;
		objCache.Region = aetObj->GetRegion();
		objCache.BlendMode = aetObj->AnimationData.BlendMode;

		if (objCache.Region != nullptr)
		{
			objCache.SpriteIndex = static_cast<int32_t>(adjustedFrame - 1);

			if (objCache.SpriteIndex >= objCache.Region->Sprites.size())
				objCache.SpriteIndex = static_cast<int32_t>(objCache.Region->Sprites.size() - 1);
			if (objCache.SpriteIndex < 0)
				objCache.SpriteIndex = 0;
		}

		Interpolate(aetObj->AnimationData, &objCache.Properties, adjustedFrame);

		AetObj* parent = aetObj->GetParent();
		if (parent != nullptr)
		{
			// TODO:
			Properties objParentProperties;
			Interpolate(parent->AnimationData, &objParentProperties, adjustedFrame);

			objCache.Properties.Origin += objParentProperties.Origin;
			objCache.Properties.Position += objParentProperties.Position;
			//objCache.Properties.Position -= objParentProperties.Origin;
			objCache.Properties.Rotation += objParentProperties.Rotation;
			objCache.Properties.Scale *= objParentProperties.Scale;
			objCache.Properties.Opacity *= objParentProperties.Opacity;
			//TransformProperties(objParentProperties, objCache.Properties);
		}

		TransformProperties(*parentProperties, objCache.Properties);
	}

	void AetMgr::InternalEffAddObjects(std::vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, float frame)
	{
		assert(aetObj->Type == AetObjType::Eff);

		if (frame < aetObj->LoopStart || frame >= aetObj->LoopEnd || !(aetObj->Flags & AetObjFlags_Visible))
			return;

		float adjustedFrame = ((frame - aetObj->LoopStart) * aetObj->PlaybackSpeed) + aetObj->StartFrame;

		Properties effProperties;
		Interpolate(aetObj->AnimationData, &effProperties, adjustedFrame);
		TransformProperties(*parentProperties, effProperties);

		AetLayer* aetLayer = aetObj->GetLayer();
		
		if (aetLayer == nullptr)
			return;
		
		for (int i = static_cast<int>(aetLayer->size() - 1); i >= 0; i--)
			InternalAddObjects(objects, &effProperties, &aetLayer->at(i), adjustedFrame);
	}
}
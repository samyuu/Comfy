#include "AetMgr.h"

namespace Graphics::Auth2D
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

	static const Properties DefaultProperites =
	{
		vec2(0.0f),	// Origin
		vec2(0.0f),	// Position
		0.0f,		// Rotation
		vec2(1.0f),	// Scale
		1.0f,		// Opacity
	};

	static void TransformProperties(const Properties& input, Properties& output)
	{
		output.Position -= input.Origin;

		if (input.Rotation != 0.0f)
		{
			float radians = glm::radians(input.Rotation);
			float sin = glm::sin(radians);
			float cos = glm::cos(radians);

			output.Position.x = output.Position.x * cos - output.Position.y * sin;
			output.Position.y = output.Position.x * sin + output.Position.y * cos;
		}

		output.Position *= input.Scale;
		output.Position += input.Position;

		output.Rotation += input.Rotation;
		output.Scale *= input.Scale;
		output.Opacity *= input.Opacity;
	}

	void AetMgr::GetAddObjects(Vector<ObjCache>& objects, const AetLayer* aetLayer, frame_t frame)
	{
		for (int i = static_cast<int>(aetLayer->size()) - 1; i >= 0; i--)
			GetAddObjects(objects, aetLayer->GetObjAt(i), frame);
	}

	void AetMgr::GetAddObjects(Vector<ObjCache>& objects, const AetObj* aetObj, frame_t frame)
	{
		Properties propreties = DefaultProperites;
		InternalAddObjects(objects, &propreties, aetObj, frame);
	}

	float AetMgr::Interpolate(const Vector<AetKeyFrame>& keyFrames, frame_t frame)
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

		float range = end->Frame - start->Frame;
		float t = (frame - start->Frame) / range;

		return (((((((t * t) * t) - ((t * t) * 2.0f)) + t) * start->Interpolation)
			+ ((((t * t) * t) - (t * t)) * end->Interpolation)) * range)
			+ (((((t * t) * 3.0f) - (((t * t) * t) * 2.0f)) * end->Value)
				+ ((((((t * t) * t) * 2.0f) - ((t * t) * 3.0f)) + 1.0f) * start->Value));
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

	AetKeyFrame* AetMgr::GetKeyFrameAt(KeyFrameCollection& keyFrames, frame_t frame)
	{
		size_t keyFrameCount = keyFrames.size();

		// NOTE: The aet editor should make sure there is always at least one KeyFrame
		assert(keyFrameCount > 0);

		// TODO: Is this the desired behavior?
		if (keyFrameCount == 1)
			return &keyFrames.front();

		for (auto& keyFrame : keyFrames)
		{
			if (keyFrame.Frame == frame)
				return &keyFrame;
		}

		// NOTE: Did the the command list get corrupted somehow?
		return nullptr;
	}

	void AetMgr::InternalAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
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

	void AetMgr::InternalPicAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
	{
		assert(aetObj->Type == AetObjType::Pic);

		if (frame < aetObj->LoopStart || frame >= aetObj->LoopEnd)
			return;

		objects.emplace_back();
		ObjCache& objCache = objects.back();

		objCache.AetObj = aetObj;
		objCache.Region = aetObj->GetReferencedRegion();
		objCache.BlendMode = aetObj->AnimationData->BlendMode;
		objCache.UseTextureMask = aetObj->AnimationData->UseTextureMask;
		objCache.Visible = aetObj->Flags.Visible;

		if (objCache.Region != nullptr && objCache.Region->SpriteCount() > 0)
		{
			// NOTE: Is it correct to modulo the index here? Seems to make more sense than just clamping
			objCache.SpriteIndex = static_cast<int>(glm::round(frame)) % objCache.Region->SpriteCount();
		}
		Interpolate(aetObj->AnimationData.get(), &objCache.Properties, frame);

		const AetObj* parent = aetObj->GetReferencedParentObj();
		if (parent != nullptr)
		{
			// TODO:
			Properties objParentProperties;
			Interpolate(parent->AnimationData.get(), &objParentProperties, frame);

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

	void AetMgr::InternalEffAddObjects(Vector<AetMgr::ObjCache>& objects, const Properties* parentProperties, const AetObj* aetObj, frame_t frame)
	{
		assert(aetObj->Type == AetObjType::Eff);

		if (frame < aetObj->LoopStart || frame >= aetObj->LoopEnd || !aetObj->Flags.Visible)
			return;

		Properties effProperties = {};
		Interpolate(aetObj->AnimationData.get(), &effProperties, frame);
		TransformProperties(*parentProperties, effProperties);

		const AetLayer* aetLayer = aetObj->GetReferencedLayer();

		if (aetLayer == nullptr)
			return;

		frame_t adjustedFrame = ((frame - aetObj->LoopStart) * aetObj->PlaybackSpeed) + aetObj->StartFrame;

		for (int i = static_cast<int>(aetLayer->size()) - 1; i >= 0; i--)
			InternalAddObjects(objects, &effProperties, aetLayer->GetObjAt(i), adjustedFrame);
	}
}
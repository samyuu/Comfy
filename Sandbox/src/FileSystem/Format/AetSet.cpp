#include "AetSet.h"
#include <assert.h>

namespace FileSystem
{
	std::array<const char*, 8> KeyFrameProperties::PropertyNames =
	{
		"Origin X",
		"Origin Y",
		"Position X",
		"Position Y",
		"Rotation",
		"Scale X",
		"Scale Y",
		"Opactiy",
	};

	std::array<const char*, 4> AetObj::TypeNames =
	{
		"nop",
		"pic",
		"aif",
		"eff",
	};

	AetSprite* AetRegion::GetSprite(int32_t index)
	{
		return (SpriteSize() > 0 && index < SpriteSize()) ? &sprites.at(index) : nullptr;
	}

	const AetSprite* AetRegion::GetSprite(int32_t index) const
	{
		return const_cast<AetRegion*>(this)->GetSprite(index);
	}

	AetSprite* AetRegion::GetFrontSprite()
	{
		return (SpriteSize() > 0) ? &sprites.front() : nullptr;
	}

	AetSprite* AetRegion::GetBackSprite()
	{
		return SpriteSize() > 0 ? &sprites.back() : nullptr;
	}

	int32_t AetRegion::SpriteSize() const
	{
		return static_cast<int32_t>(sprites.size());
	}

	SpriteCollection& AetRegion::GetSprites()
	{
		return sprites;
	}

	const SpriteCollection& AetRegion::GetSprites() const
	{
		return sprites;
	}

	AetMarker::AetMarker()
	{
	}

	AetMarker::AetMarker(frame_t frame, const std::string& name) : Frame(frame), Name(name)
	{
	}

	AetKeyFrame::AetKeyFrame() : AetKeyFrame(0.0f, 0.0f, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(float value) : AetKeyFrame(0.0f, value, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(frame_t frame, float value, float interpolation) : Frame(frame), Value(value), Interpolation(interpolation)
	{
	}

	AetObj::AetObj()
	{
	}

	AetObj::AetObj(AetObjType type, const std::string& name, AetLayer* parentLayer)
	{
		LoopStart = 0.0f;
		LoopEnd = 60.0f;
		StartFrame = 0.0f;
		PlaybackSpeed = 1.0f;
		Flags.Visible = true;
		Flags.Audible = true;
		TypePaddingByte = 0x3;
		Type = type;

		if (type != AetObjType::Aif)
		{
			AnimationData = MakeRefPtr<FileSystem::AnimationData>();
			AnimationData->BlendMode = AetBlendMode::Alpha;
			AnimationData->UseTextureMask = false;
			AnimationData->Properties.OriginX().emplace_back(0.0f);
			AnimationData->Properties.OriginY().emplace_back(0.0f);
			AnimationData->Properties.PositionX().emplace_back(0.0f);
			AnimationData->Properties.PositionY().emplace_back(0.0f);
			AnimationData->Properties.Rotation().emplace_back(0.0f);
			AnimationData->Properties.ScaleX().emplace_back(1.0f);
			AnimationData->Properties.ScaleY().emplace_back(1.0f);
			AnimationData->Properties.Opacity().emplace_back(1.0f);
		}

		this->parentLayer = parentLayer;

		SetName(name);
	}

	const std::string& AetObj::GetName() const
	{
		return name;
	}

	void AetObj::SetName(const char* value)
	{
		name = value;
		assert(parentLayer != nullptr && parentLayer->GetParentAet() != nullptr);
		GetParentAet()->InternalUpdateLayerNames();
	}

	void AetObj::SetName(const std::string& value)
	{
		SetName(value.c_str());
	}

	AetRegion* AetObj::GetReferencedRegion()
	{
		return references.Region.get();
	}

	const AetRegion* AetObj::GetReferencedRegion() const
	{
		return references.Region.get();
	}

	void AetObj::SetReferencedRegion(const RefPtr<AetRegion>& value)
	{
		references.Region = value;
	}

	AetSoundEffect* AetObj::GetReferencedSoundEffect()
	{
		return references.SoundEffect.get();
	}

	const AetSoundEffect* AetObj::GetReferencedSoundEffect() const
	{
		return references.SoundEffect.get();
	}

	void AetObj::SetReferencedSoundEffect(const RefPtr<AetSoundEffect>& value)
	{
		references.SoundEffect = value;
	}

	AetLayer* AetObj::GetReferencedLayer()
	{
		return references.Layer.get();
	}

	const AetLayer* AetObj::GetReferencedLayer() const
	{
		return references.Layer.get();
	}

	void AetObj::SetReferencedLayer(const RefPtr<AetLayer>& value)
	{
		references.Layer = value;
	}

	AetObj* AetObj::GetReferebcedParentObj()
	{
		return references.ParentObj.get();
	}

	const AetObj* AetObj::GetReferebcedParentObj() const
	{
		return references.ParentObj.get();
	}

	void AetObj::SetParentObj(const RefPtr<AetObj>& value)
	{
		references.ParentObj = value;
	}

	Aet* AetObj::GetParentAet()
	{
		assert(parentLayer != nullptr);
		return parentLayer->GetParentAet();
	}

	const Aet* AetObj::GetParentAet() const
	{
		assert(parentLayer != nullptr);
		return parentLayer->GetParentAet();
	}

	AetLayer* AetObj::GetParentLayer()
	{
		return parentLayer;
	}

	const AetLayer* AetObj::GetParentLayer() const
	{
		return parentLayer;
	}

	AetObj* AetLayer::GetObj(const std::string& name)
	{
		for (int32_t i = 0; i < size(); i++)
		{
			if (objects[i]->GetName() == name)
				return objects[i].get();
		}

		return nullptr;
	}

	const AetObj* AetLayer::GetObj(const std::string& name) const
	{
		return const_cast<AetLayer*>(this)->GetObj(name);
	}

	const std::vector<std::string>& AetLayer::GetGivenNames() const
	{
		return givenNames;
	}

	const char* AetLayer::GetCommaSeparatedNames() const
	{
		return commaSeparatedNames.c_str();
	}

	void AetLayer::AddNewObject(AetObjType type, const std::string& name)
	{
		objects.push_back(MakeRefPtr<AetObj>(type, name, this));
	}

	void AetLayer::DeleteObject(AetObj* object)
	{
		int index = 0;
		for (RefPtr<AetObj>& obj : objects)
		{
			if (obj.get() == object)
			{
				objects.erase(objects.begin() + index);
				break;
			}

			index++;
		}
	}

	AetLayer* Aet::GetRootLayer()
	{
		return AetLayers.size() > 0 ? AetLayers.back().get() : nullptr;
	}

	AetObj* Aet::GetObj(const std::string& name)
	{
		for (int32_t i = static_cast<int32_t>(AetLayers.size()) - 1; i >= 0; i--)
		{
			AetObj* obj = AetLayers[i]->GetObj(name);
			if (obj != nullptr)
				return obj;
		}

		return nullptr;
	}

	const AetObj* Aet::GetObj(const std::string& name) const
	{
		return const_cast<AetObj*>(const_cast<Aet*>(this)->GetObj(name));
	}

	int32_t Aet::GetObjIndex(AetLayer& layer, const std::string& name) const
	{
		for (int32_t i = static_cast<int32_t>(layer.size()) - 1; i >= 0; i--)
		{
			if (layer[i]->GetName() == name)
				return i;
		}

		return -1;
	}

	void Aet::UpdateParentPointers()
	{
		for (RefPtr<AetLayer>& layer : AetLayers)
		{
			layer->parentAet = this;

			for (RefPtr<AetObj>& obj : *layer)
				obj->parentLayer = layer.get();
		}
	}

	void Aet::InternalUpdateLayerNames()
	{
		AetLayer* rootLayer = GetRootLayer();

		for (RefPtr<AetLayer>& aetLayer : AetLayers)
		{
			aetLayer->givenNames.clear();

			for (RefPtr<AetObj>& aetObj : *aetLayer)
			{
				const AetLayer* referencedLayer;

				if (aetObj->Type == AetObjType::Eff && (referencedLayer = aetObj->GetReferencedLayer()) != nullptr)
				{
					bool nameExists = false;
					for (auto& layerNames : referencedLayer->givenNames)
					{
						if (layerNames == aetObj->GetName())
						{
							nameExists = true;
							break;
						}
					}

					if (!nameExists)
						const_cast<AetLayer*>(referencedLayer)->givenNames.emplace_back(aetObj->GetName());
				}
			}

			if (aetLayer.get() == rootLayer)
				aetLayer->givenNames.emplace_back("Root");
		}

		for (RefPtr<AetLayer>& aetLayer : AetLayers)
		{
			aetLayer->commaSeparatedNames.clear();

			for (size_t i = 0; i < aetLayer->givenNames.size(); i++)
			{
				aetLayer->commaSeparatedNames.append(aetLayer->givenNames[i]);
				if (i < aetLayer->givenNames.size() - 1)
					aetLayer->commaSeparatedNames.append(", ");
			}
		}
	}

	void Aet::InternalLinkPostRead()
	{
		int32_t layerIndex = 0;
		for (RefPtr<AetLayer>& aetLayer : AetLayers)
		{
			aetLayer->thisIndex = layerIndex++;

			for (RefPtr<AetObj>& aetObj : *aetLayer)
			{
				if (aetObj->dataFilePtr != nullptr)
				{
					if (aetObj->Type == AetObjType::Pic)
						InternalFindObjReferencedRegion(aetObj.get());
					else if (aetObj->Type == AetObjType::Aif)
						InternalFindObjReferencedSoundEffect(aetObj.get());
					else if (aetObj->Type == AetObjType::Eff)
						InternalFindObjReferencedLayer(aetObj.get());
				}
				if (aetObj->parentFilePtr != nullptr)
				{
					InternalFindObjReferencedParent(aetObj.get());
				}
			}
		}
	}

	void Aet::InternalFindObjReferencedRegion(AetObj* aetObj)
	{
		for (RefPtr<AetRegion>& otherRegion : AetRegions)
		{
			if (otherRegion->filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.Region = otherRegion;
				return;
			}
		}
	}

	void Aet::InternalFindObjReferencedSoundEffect(AetObj* aetObj)
	{
		for (RefPtr<AetSoundEffect>& otherSoundEffect : AetSoundEffects)
		{
			if (otherSoundEffect->filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.SoundEffect = otherSoundEffect;
				return;
			}
		}
	}

	void Aet::InternalFindObjReferencedLayer(AetObj* aetObj)
	{
		for (RefPtr<AetLayer>& otherLayer : AetLayers)
		{
			if (otherLayer->filePosition == aetObj->dataFilePtr)
			{
				aetObj->references.Layer = otherLayer;
				return;
			}
		}
	}

	void Aet::InternalFindObjReferencedParent(AetObj* aetObj)
	{
		for (RefPtr<AetLayer>& otherLayer : AetLayers) for (RefPtr<AetObj>& otherObj : *otherLayer)
		{
			if (otherObj->filePosition == aetObj->parentFilePtr)
			{
				aetObj->references.ParentObj = otherObj;
				return;
			}
		}
	}

	void AetSet::ClearSpriteCache()
	{
		for (RefPtr<Aet>& aet : aets)
		{
			for (RefPtr<AetRegion>& region : aet->AetRegions)
			{
				for (AetSprite& sprite : region->GetSprites())
					sprite.SpriteCache = nullptr;
			}
		}
	}
}
#include "AetSet.h"
#include <assert.h>

namespace FileSystem
{
	Array<const char*, 8> KeyFrameProperties::PropertyNames =
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

	Array<const char*, 13> AnimationData::BlendModeNames =
	{
		nullptr,
		nullptr,
		nullptr,
		"Alpha",
		nullptr,
		"Additive",
		"Destination Color Zero",
		"Source Alpha One Minus Source Color",
		"Transparent",
		nullptr,
		nullptr,
		nullptr,
		nullptr, // "What The Fuck?"
	};

	Array<const char*, 4> AetObj::TypeNames =
	{
		"nop",
		"pic",
		"aif",
		"eff",
	};

	const char* AnimationData::GetBlendModeName(AetBlendMode blendMode)
	{
		size_t blendModeIndex = static_cast<size_t>(blendMode);

		// NOTE: This should never happen
		if (blendModeIndex >= BlendModeNames.size())
			return "Invalid Blend Mode";

		const char* name = BlendModeNames[blendModeIndex];
		return (name == nullptr) ? "Undefined Blend Mode" : name;
	}

	AetSprite* AetRegion::GetSprite(int32_t index)
	{
		if (SpriteCount() < 1)
			return nullptr;

		assert(index >= 0 && index < SpriteCount());
		return &sprites[index];
	}

	const AetSprite* AetRegion::GetSprite(int32_t index) const
	{
		return const_cast<AetRegion*>(this)->GetSprite(index);
	}

	AetSprite* AetRegion::GetFrontSprite()
	{
		return (SpriteCount() > 0) ? &sprites.front() : nullptr;
	}

	AetSprite* AetRegion::GetBackSprite()
	{
		return SpriteCount() > 0 ? &sprites.back() : nullptr;
	}

	int32_t AetRegion::SpriteCount() const
	{
		return static_cast<int32_t>(sprites.size());
	}

	Vector<AetSprite>& AetRegion::GetSprites()
	{
		return sprites;
	}

	const Vector<AetSprite>& AetRegion::GetSprites() const
	{
		return sprites;
	}

	AetMarker::AetMarker()
	{
	}

	AetMarker::AetMarker(frame_t frame, const String& name) : Frame(frame), Name(name)
	{
	}

	AetKeyFrame::AetKeyFrame() : AetKeyFrame(0.0f, 0.0f, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(float value) : AetKeyFrame(0.0f, value, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(frame_t frame, float value) : AetKeyFrame(frame, value, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(frame_t frame, float value, float interpolation) : Frame(frame), Value(value), Interpolation(interpolation)
	{
	}

	AetObj::AetObj()
	{
	}

	AetObj::AetObj(AetObjType type, const String& name, AetLayer* parentLayer)
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
			AnimationData = MakeRef<FileSystem::AnimationData>();
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

	const String& AetObj::GetName() const
	{
		return name;
	}

	void AetObj::SetName(const char* value)
	{
		name = value;
		assert(parentLayer != nullptr && parentLayer->GetParentAet() != nullptr);
		GetParentAet()->InternalUpdateLayerNames();
	}

	void AetObj::SetName(const String& value)
	{
		SetName(value.c_str());
	}

	bool AetObj::GetIsVisible() const
	{
		return Flags.Visible;
	}

	void AetObj::SetIsVisible(bool value)
	{
		Flags.Visible = value;
	}

	bool AetObj::GetIsAudible() const
	{
		return Flags.Audible;
	}

	void AetObj::SetIsAudible(bool value)
	{
		Flags.Audible = value;
	}

	const RefPtr<AetRegion>& AetObj::GetReferencedRegion()
	{
		return references.Region;
	}

	const AetRegion* AetObj::GetReferencedRegion() const
	{
		return references.Region.get();
	}

	void AetObj::SetReferencedRegion(const RefPtr<AetRegion>& value)
	{
		references.Region = value;
	}

	const RefPtr<AetSoundEffect>& AetObj::GetReferencedSoundEffect()
	{
		return references.SoundEffect;
	}

	const AetSoundEffect* AetObj::GetReferencedSoundEffect() const
	{
		return references.SoundEffect.get();
	}

	void AetObj::SetReferencedSoundEffect(const RefPtr<AetSoundEffect>& value)
	{
		references.SoundEffect = value;
	}

	const RefPtr<AetLayer>& AetObj::GetReferencedLayer()
	{
		return references.Layer;
	}

	const AetLayer* AetObj::GetReferencedLayer() const
	{
		return references.Layer.get();
	}

	void AetObj::SetReferencedLayer(const RefPtr<AetLayer>& value)
	{
		references.Layer = value;
	}

	const RefPtr<AetObj>& AetObj::GetReferencedParentObj()
	{
		return references.ParentObj;
	}

	const AetObj* AetObj::GetReferencedParentObj() const
	{
		return references.ParentObj.get();
	}

	void AetObj::SetReferencedParentObj(const RefPtr<AetObj>& value)
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

	RefPtr<AetObj> AetLayer::FindObj(const String& name)
	{
		for (int32_t i = 0; i < size(); i++)
		{
			if (objects[i]->GetName() == name)
				return objects[i];
		}

		return nullptr;
	}

	RefPtr<const AetObj> AetLayer::FindObj(const String& name) const
	{
		return const_cast<AetLayer*>(this)->FindObj(name);
	}

	const Vector<String>& AetLayer::GetGivenNames() const
	{
		return givenNames;
	}

	const String AetLayer::unusedLayerName = "Unused";

	const String& AetLayer::GetCommaSeparatedNames() const
	{
		if (commaSeparatedNames.empty())
			return unusedLayerName;

		return commaSeparatedNames;
	}

	void AetLayer::AddNewObject(AetObjType type, const String& name)
	{
		objects.push_back(MakeRef<AetObj>(type, name, this));
	}

	void AetLayer::DeleteObject(AetObj* value)
	{
		int index = 0;
		for (RefPtr<AetObj>& obj : objects)
		{
			if (obj.get() == value)
			{
				objects.erase(objects.begin() + index);
				break;
			}

			index++;
		}

		parentAet->InternalUpdateLayerNames();
	}

	AetLayer* Aet::GetRootLayer()
	{
		return RootLayer.get();
	}

	RefPtr<AetObj> Aet::FindObj(const String& name)
	{
		const RefPtr<AetObj>& rootFoundObj = RootLayer->FindObj(name);
		if (rootFoundObj != nullptr)
			return rootFoundObj;

		for (int32_t i = static_cast<int32_t>(Layers.size()) - 1; i >= 0; i--)
		{
			const RefPtr<AetObj>& obj = Layers[i]->FindObj(name);
			if (obj != nullptr)
				return obj;
		}

		return nullptr;
	}

	RefPtr<const AetObj> Aet::FindObj(const String& name) const
	{
		return const_cast<Aet*>(this)->FindObj(name);
	}

	int32_t Aet::FindObjIndex(AetLayer& layer, const String& name) const
	{
		for (int32_t i = static_cast<int32_t>(layer.size()) - 1; i >= 0; i--)
		{
			if (layer[i]->GetName() == name)
				return i;
		}

		return -1;
	}

	// TODO:
	/*
	void Aet::DeleteLayer(const RefPtr<AetLayer>& value)
	{
		int index = 0;
		for (RefPtr<AetLayer>& layer : Layers)
		{
			if (layer == value)
			{
				Layers.erase(Layers.begin() + index);
				break;
			}

			index++;
		}

		for (RefPtr<AetLayer>& layer : Layers)
		{
			for (RefPtr<AetObj>& obj : *layer)
			{
				if (obj->GetReferencedLayer() == value)
				{
					// TODO: maybe store them in a separate "lastDeleted" field to easily recover for undo
					obj->SetReferencedLayer(nullptr);
				}
			}
		}

		InternalUpdateLayerNames();
	}
	*/

	void Aet::UpdateParentPointers()
	{
		const auto updateParentPointers = [this](RefPtr<AetLayer>& layer)
		{
			layer->parentAet = this;

			for (RefPtr<AetObj>& obj : *layer)
				obj->parentLayer = layer.get();
		};

		for (RefPtr<AetLayer>& layer : Layers)
			updateParentPointers(layer);

		updateParentPointers(RootLayer);
	}

	void Aet::InternalUpdateLayerNames()
	{
		for (RefPtr<AetLayer>& aetLayer : Layers)
			InternalUpdateLayerNamesVector(aetLayer);
		
		InternalUpdateLayerNamesVector(RootLayer);
		RootLayer->givenNames = { "Root Layer" };

		for (RefPtr<AetLayer>& aetLayer : Layers)
			InternalUpdateLayerNamesCommaSeparated(aetLayer);

		InternalUpdateLayerNamesCommaSeparated(RootLayer);
	}

	void Aet::InternalUpdateLayerNamesVector(RefPtr<AetLayer>& aetLayer)
	{
		aetLayer->givenNames.clear();

		for (RefPtr<AetObj>& aetObj : *aetLayer)
		{
			AetLayer* referencedLayer;

			if (aetObj->Type == AetObjType::Eff && (referencedLayer = aetObj->GetReferencedLayer().get()) != nullptr)
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
					referencedLayer->givenNames.emplace_back(aetObj->GetName());
			}
		}
	}

	void Aet::InternalUpdateLayerNamesCommaSeparated(RefPtr<AetLayer>& aetLayer)
	{
		aetLayer->commaSeparatedNames.clear();

		for (size_t i = 0; i < aetLayer->givenNames.size(); i++)
		{
			aetLayer->commaSeparatedNames.append(aetLayer->givenNames[i]);
			if (i < aetLayer->givenNames.size() - 1)
				aetLayer->commaSeparatedNames.append(", ");
		}
	}

	void Aet::InternalLinkPostRead()
	{
		assert(RootLayer != nullptr);

		for (RefPtr<AetLayer>& aetLayer : Layers)
			InternalLinkeLayerContent(aetLayer);

		InternalLinkeLayerContent(RootLayer);
	}

	void Aet::InternalLinkeLayerContent(RefPtr<AetLayer>& aetLayer)
	{
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

	void Aet::InternalFindObjReferencedRegion(AetObj* aetObj)
	{
		for (RefPtr<AetRegion>& otherRegion : Regions)
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
		for (RefPtr<AetSoundEffect>& otherSoundEffect : SoundEffects)
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
		for (RefPtr<AetLayer>& otherLayer : Layers)
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
		for (RefPtr<AetLayer>& otherLayer : Layers)
		{
			for (RefPtr<AetObj>& otherObj : *otherLayer)
			{
				if (otherObj->filePosition == aetObj->parentFilePtr)
				{
					aetObj->references.ParentObj = otherObj;
					return;
				}
			}
		}
	}

	void AetSet::ClearSpriteCache()
	{
		for (RefPtr<Aet>& aet : aets)
		{
			for (RefPtr<AetRegion>& region : aet->Regions)
			{
				for (AetSprite& sprite : region->GetSprites())
					sprite.SpriteCache = nullptr;
			}
		}
	}
}
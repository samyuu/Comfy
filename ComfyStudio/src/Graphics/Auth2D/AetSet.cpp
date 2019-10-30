#include "AetSet.h"
#include <assert.h>

namespace Graphics
{
	const std::array<const char*, 8> KeyFrameProperties::PropertyNames =
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

	const std::array<const char*, 13> AnimationData::BlendModeNames =
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

	const std::array<const char*, 4> AetObj::TypeNames =
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

	AetSpriteIdentifier* AetRegion::GetSprite(int32_t index)
	{
		if (SpriteCount() < 1)
			return nullptr;

		assert(index >= 0 && index < SpriteCount());
		return &sprites[index];
	}

	const AetSpriteIdentifier* AetRegion::GetSprite(int32_t index) const
	{
		return const_cast<AetRegion*>(this)->GetSprite(index);
	}

	AetSpriteIdentifier* AetRegion::GetFrontSprite()
	{
		return (SpriteCount() > 0) ? &sprites.front() : nullptr;
	}

	AetSpriteIdentifier* AetRegion::GetBackSprite()
	{
		return SpriteCount() > 0 ? &sprites.back() : nullptr;
	}

	int32_t AetRegion::SpriteCount() const
	{
		return static_cast<int32_t>(sprites.size());
	}

	std::vector<AetSpriteIdentifier>& AetRegion::GetSprites()
	{
		return sprites;
	}

	const std::vector<AetSpriteIdentifier>& AetRegion::GetSprites() const
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

	AetKeyFrame::AetKeyFrame(frame_t frame, float value) : AetKeyFrame(frame, value, 0.0f)
	{
	}

	AetKeyFrame::AetKeyFrame(frame_t frame, float value, float curve) : Frame(frame), Value(value), Curve(curve)
	{
	}

	AetObj::AetObj()
	{
	}

	AetObj::AetObj(AetObjType type, const std::string& name, AetLayer* parentLayer)
	{
		StartFrame = 0.0f;
		EndFrame = 60.0f;
		StartOffset = 0.0f;
		PlaybackSpeed = 1.0f;
		Flags.Visible = true;
		Flags.Audible = true;
		TypePaddingByte = 0x3;
		Type = type;

		if (type != AetObjType::Aif)
		{
			AnimationData = MakeRef<Graphics::AnimationData>();
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

	AetObj::~AetObj()
	{
	}

	const std::string& AetObj::GetName() const
	{
		return name;
	}

	void AetObj::SetName(const std::string& value)
	{
		name = value;
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

	Aet* AetLayer::GetParentAet() const
	{
		return parentAet;
	}

	bool AetLayer::IsRootLayer() const
	{
		return this == parentAet->RootLayer.get();
	}

	const std::string& AetLayer::GetName() const
	{
		return givenName;
	}

	void AetLayer::SetName(const std::string& value)
	{
		givenName = value;
	}

	RefPtr<AetObj> AetLayer::FindObj(const std::string& name)
	{
		for (int32_t i = 0; i < size(); i++)
		{
			if (objects[i]->GetName() == name)
				return objects[i];
		}

		return nullptr;
	}

	RefPtr<const AetObj> AetLayer::FindObj(const std::string& name) const
	{
		return const_cast<AetLayer*>(this)->FindObj(name);
	}

	const std::string AetLayer::rootLayerName = "Root Layer";
	const std::string AetLayer::unusedLayerName = "Unused Layer";

	void AetLayer::AddNewObject(AetObjType type, const std::string& name)
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
	}

	AetLayer* Aet::GetRootLayer()
	{
		return RootLayer.get();
	}

	const AetLayer* Aet::GetRootLayer() const
	{
		return RootLayer.get();
	}

	RefPtr<AetObj> Aet::FindObj(const std::string& name)
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

	RefPtr<const AetObj> Aet::FindObj(const std::string& name) const
	{
		return const_cast<Aet*>(this)->FindObj(name);
	}

	int32_t Aet::FindObjIndex(AetLayer& layer, const std::string& name) const
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

	void Aet::InternalUpdateLayerNamesAfteObjectReferences()
	{
		RootLayer->SetName(AetLayer::rootLayerName);
		InternalUpdateLayerNamesAfteObjectReferences(RootLayer);
		
		for (RefPtr<AetLayer>& aetLayer : Layers)
			InternalUpdateLayerNamesAfteObjectReferences(aetLayer);
	}

	void Aet::InternalUpdateLayerNamesAfteObjectReferences(RefPtr<AetLayer>& aetLayer)
	{
		for (RefPtr<AetObj>& aetObj : *aetLayer)
		{
			if (aetObj->Type == AetObjType::Eff)
			{
				AetLayer* referencedLayer = aetObj->GetReferencedLayer().get();

				if (referencedLayer != nullptr)
					referencedLayer->SetName(aetObj->GetName());
			}
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
				for (AetSpriteIdentifier& sprite : region->GetSprites())
					sprite.SpriteCache = nullptr;
			}
		}
	}
}
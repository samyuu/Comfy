#include "AetSet.h"
#include <assert.h>

namespace Graphics
{
	const std::array<const char*, 8> AetTransform::FieldNames =
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

	const std::array<const char*, 4> AetLayer::TypeNames =
	{
		"nop",
		"pic",
		"aif",
		"eff",
	};

	AetSpriteIdentifier* AetSurface::GetSprite(int32_t index)
	{
		if (SpriteCount() < 1 || index < 0 || index >= SpriteCount())
			return nullptr;

		return &sprites[index];
	}

	const AetSpriteIdentifier* AetSurface::GetSprite(int32_t index) const
	{
		return const_cast<AetSurface*>(this)->GetSprite(index);
	}

	AetSpriteIdentifier* AetSurface::GetFrontSprite()
	{
		return (SpriteCount() > 0) ? &sprites.front() : nullptr;
	}

	AetSpriteIdentifier* AetSurface::GetBackSprite()
	{
		return SpriteCount() > 0 ? &sprites.back() : nullptr;
	}

	int32_t AetSurface::SpriteCount() const
	{
		return static_cast<int32_t>(sprites.size());
	}

	std::vector<AetSpriteIdentifier>& AetSurface::GetSprites()
	{
		return sprites;
	}

	const std::vector<AetSpriteIdentifier>& AetSurface::GetSprites() const
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

	AetLayer::AetLayer()
	{
	}

	AetLayer::AetLayer(AetLayerType type, const std::string& name, AetComposition* parentComp)
	{
		StartFrame = 0.0f;
		EndFrame = 60.0f;
		StartOffset = 0.0f;
		PlaybackSpeed = 1.0f;
		Flags.Visible = true;
		Flags.Audible = true;
		TypePaddingByte = 0x3;
		Type = type;

		if (type != AetLayerType::Aif)
		{
			AnimationData = MakeRef<Graphics::AetAnimationData>();
			AnimationData->BlendMode = AetBlendMode::Normal;
			AnimationData->UseTextureMask = false;

			AnimationData->Transform.Origin.X->emplace_back(0.0f);
			AnimationData->Transform.Origin.Y->emplace_back(0.0f);
			AnimationData->Transform.Position.X->emplace_back(0.0f);
			AnimationData->Transform.Position.Y->emplace_back(0.0f);
			AnimationData->Transform.Rotation->emplace_back(0.0f);
			AnimationData->Transform.Scale.X->emplace_back(0.0f);
			AnimationData->Transform.Scale.Y->emplace_back(0.0f);
			AnimationData->Transform.Opacity->emplace_back(0.0f);
		}

		this->parentComposition = parentComp;

		SetName(name);
	}

	AetLayer::~AetLayer()
	{
	}

	const std::string& AetLayer::GetName() const
	{
		return name;
	}

	void AetLayer::SetName(const std::string& value)
	{
		name = value;
	}

	bool AetLayer::GetIsVisible() const
	{
		return Flags.Visible;
	}

	void AetLayer::SetIsVisible(bool value)
	{
		Flags.Visible = value;
	}

	bool AetLayer::GetIsAudible() const
	{
		return Flags.Audible;
	}

	void AetLayer::SetIsAudible(bool value)
	{
		Flags.Audible = value;
	}

	const RefPtr<AetSurface>& AetLayer::GetReferencedSurface()
	{
		return references.Surface;
	}

	const AetSurface* AetLayer::GetReferencedSurface() const
	{
		return references.Surface.get();
	}

	void AetLayer::SetReferencedSurface(const RefPtr<AetSurface>& value)
	{
		references.Surface = value;
	}

	const RefPtr<AetSoundEffect>& AetLayer::GetReferencedSoundEffect()
	{
		return references.SoundEffect;
	}

	const AetSoundEffect* AetLayer::GetReferencedSoundEffect() const
	{
		return references.SoundEffect.get();
	}

	void AetLayer::SetReferencedSoundEffect(const RefPtr<AetSoundEffect>& value)
	{
		references.SoundEffect = value;
	}

	const RefPtr<AetComposition>& AetLayer::GetReferencedComposition()
	{
		return references.Composition;
	}

	const AetComposition* AetLayer::GetReferencedComposition() const
	{
		return references.Composition.get();
	}

	void AetLayer::SetReferencedComposition(const RefPtr<AetComposition>& value)
	{
		references.Composition = value;
	}

	const RefPtr<AetLayer>& AetLayer::GetReferencedParentLayer()
	{
		return references.ParentLayer;
	}

	const AetLayer* AetLayer::GetReferencedParentLayer() const
	{
		return references.ParentLayer.get();
	}

	void AetLayer::SetReferencedParentLayer(const RefPtr<AetLayer>& value)
	{
		references.ParentLayer = value;
	}

	Aet* AetLayer::GetParentAet()
	{
		assert(parentComposition != nullptr);
		return parentComposition->GetParentAet();
	}

	const Aet* AetLayer::GetParentAet() const
	{
		assert(parentComposition != nullptr);
		return parentComposition->GetParentAet();
	}

	AetComposition* AetLayer::GetParentComposition()
	{
		return parentComposition;
	}

	const AetComposition* AetLayer::GetParentComposition() const
	{
		return parentComposition;
	}

	Aet* AetComposition::GetParentAet() const
	{
		return parentAet;
	}

	bool AetComposition::IsRootComposition() const
	{
		return this == parentAet->RootComposition.get();
	}

	const std::string& AetComposition::GetName() const
	{
		return givenName;
	}

	void AetComposition::SetName(const std::string& value)
	{
		givenName = value;
	}

	RefPtr<AetLayer> AetComposition::FindLayer(const std::string& name)
	{
		for (int32_t i = 0; i < size(); i++)
		{
			if (layers[i]->GetName() == name)
				return layers[i];
		}

		return nullptr;
	}

	RefPtr<const AetLayer> AetComposition::FindLayer(const std::string& name) const
	{
		return const_cast<AetComposition*>(this)->FindLayer(name);
	}

	const std::string AetComposition::rootCompositionName = "Root";
	const std::string AetComposition::unusedCompositionName = "Unused Comp";

	void AetComposition::AddNewLayer(AetLayerType type, const std::string& name)
	{
		layers.push_back(MakeRef<AetLayer>(type, name, this));
	}

	void AetComposition::DeleteLayer(AetLayer* value)
	{
		int index = 0;
		for (RefPtr<AetLayer>& layer : layers)
		{
			if (layer.get() == value)
			{
				layers.erase(layers.begin() + index);
				break;
			}

			index++;
		}
	}

	AetComposition* Aet::GetRootComposition()
	{
		return RootComposition.get();
	}

	const AetComposition* Aet::GetRootComposition() const
	{
		return RootComposition.get();
	}

	RefPtr<AetLayer> Aet::FindLayer(const std::string& name)
	{
		const RefPtr<AetLayer>& rootFoundLayer = RootComposition->FindLayer(name);
		if (rootFoundLayer != nullptr)
			return rootFoundLayer;

		for (int32_t i = static_cast<int32_t>(Compositions.size()) - 1; i >= 0; i--)
		{
			const RefPtr<AetLayer>& layer = Compositions[i]->FindLayer(name);
			if (layer != nullptr)
				return layer;
		}

		return nullptr;
	}

	RefPtr<const AetLayer> Aet::FindLayer(const std::string& name) const
	{
		return const_cast<Aet*>(this)->FindLayer(name);
	}

	int32_t Aet::FindLayerIndex(AetComposition& comp, const std::string& name) const
	{
		for (int32_t i = static_cast<int32_t>(comp.size()) - 1; i >= 0; i--)
		{
			if (comp[i]->GetName() == name)
				return i;
		}

		return -1;
	}

	// TODO:
	/*
	void Aet::DeleteComposition(const RefPtr<AetComposition>& value)
	{
		int index = 0;
		for (RefPtr<AetComposition>& comp : Compositions)
		{
			if (comp == value)
			{
				Compositionss.erase(Compositions.begin() + index);
				break;
			}

			index++;
		}

		for (RefPtr<AetComposition>& comp : Compositions)
		{
			for (RefPtr<AetLayer>& layer : *comp)
			{
				if (layer->GetReferencedComposition() == value)
				{
					// TODO: maybe store them in a separate "lastDeleted" field to easily recover for undo
					layer->SetReferencedComposition(nullptr);
				}
			}
		}

		InternalUpdateCompositionNames();
	}
	*/

	void Aet::UpdateParentPointers()
	{
		const auto updateParentPointers = [this](RefPtr<AetComposition>& comp)
		{
			comp->parentAet = this;

			for (RefPtr<AetLayer>& layer : *comp)
				layer->parentComposition = comp.get();
		};

		for (RefPtr<AetComposition>& comp : Compositions)
			updateParentPointers(comp);

		updateParentPointers(RootComposition);
	}

	void Aet::InternalUpdateCompositionNamesAfterLayerReferences()
	{
		RootComposition->SetName(AetComposition::rootCompositionName);
		InternalUpdateCompositionNamesAfterLayerReferences(RootComposition);
		
		for (RefPtr<AetComposition>& comp : Compositions)
			InternalUpdateCompositionNamesAfterLayerReferences(comp);
	}

	void Aet::InternalUpdateCompositionNamesAfterLayerReferences(RefPtr<AetComposition>& comp)
	{
		for (RefPtr<AetLayer>& layer : *comp)
		{
			if (layer->Type == AetLayerType::Eff)
			{
				AetComposition* referencedComp = layer->GetReferencedComposition().get();

				if (referencedComp != nullptr)
					referencedComp->SetName(layer->GetName());
			}
		}
	}

	void Aet::InternalLinkPostRead()
	{
		assert(RootComposition != nullptr);

		for (RefPtr<AetComposition>& comp : Compositions)
			InternalLinkeCompositionContent(comp);

		InternalLinkeCompositionContent(RootComposition);
	}

	void Aet::InternalLinkeCompositionContent(RefPtr<AetComposition>& comp)
	{
		for (RefPtr<AetLayer>& layer : *comp)
		{
			if (layer->dataFilePtr != FileAddr::NullPtr)
			{
				if (layer->Type == AetLayerType::Pic)
					InternalFindLayerReferencedSurface(layer.get());
				else if (layer->Type == AetLayerType::Aif)
					InternalFindLayerReferencedSoundEffect(layer.get());
				else if (layer->Type == AetLayerType::Eff)
					InternalFindLayerReferencedComposition(layer.get());
			}
			if (layer->parentFilePtr != FileAddr::NullPtr)
			{
				InternalFindLayerReferencedParent(layer.get());
			}
		}
	}

	void Aet::InternalFindLayerReferencedSurface(AetLayer* layer)
	{
		for (RefPtr<AetSurface>& otherSurfaces : Surfaces)
		{
			if (otherSurfaces->filePosition == layer->dataFilePtr)
			{
				layer->references.Surface = otherSurfaces;
				return;
			}
		}
	}

	void Aet::InternalFindLayerReferencedSoundEffect(AetLayer* layer)
	{
		for (RefPtr<AetSoundEffect>& otherSoundEffect : SoundEffects)
		{
			if (otherSoundEffect->filePosition == layer->dataFilePtr)
			{
				layer->references.SoundEffect = otherSoundEffect;
				return;
			}
		}
	}

	void Aet::InternalFindLayerReferencedComposition(AetLayer* layer)
	{
		for (RefPtr<AetComposition>& otherComp : Compositions)
		{
			if (otherComp->filePosition == layer->dataFilePtr)
			{
				layer->references.Composition = otherComp;
				return;
			}
		}
	}

	void Aet::InternalFindLayerReferencedParent(AetLayer* layer)
	{
		for (RefPtr<AetComposition>& otherComp : Compositions)
		{
			for (RefPtr<AetLayer>& otherLayer : *otherComp)
			{
				if (otherLayer->filePosition == layer->parentFilePtr)
				{
					layer->references.ParentLayer = otherLayer;
					return;
				}
			}
		}
	}

	void AetSet::ClearSpriteCache()
	{
		for (RefPtr<Aet>& aet : aets)
		{
			for (RefPtr<AetSurface>& surface : aet->Surfaces)
			{
				for (AetSpriteIdentifier& sprite : surface->GetSprites())
					sprite.SpriteCache = nullptr;
			}
		}
	}
}

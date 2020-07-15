#include "AetSet.h"

namespace Comfy::Graphics::Aet
{
	VideoSource* Video::GetSource(int index)
	{
		return IndexOrNull(index, Sources);
	}

	const VideoSource* Video::GetSource(int index) const
	{
		return IndexOrNull(index, Sources);
	}

	VideoSource* Video::GetFront()
	{
		return (!Sources.empty()) ? &Sources.front() : nullptr;
	}

	VideoSource* Video::GetBack()
	{
		return (!Sources.empty()) ? &Sources.back() : nullptr;
	}

	const std::string& Layer::GetName() const
	{
		return name;
	}

	void Layer::SetName(const std::string& value)
	{
		name = value;
	}

	bool Layer::GetIsVisible() const
	{
		return Flags.VideoActive;
	}

	void Layer::SetIsVisible(bool value)
	{
		Flags.VideoActive = value;
	}

	bool Layer::GetIsAudible() const
	{
		return Flags.AudioActive;
	}

	void Layer::SetIsAudible(bool value)
	{
		Flags.AudioActive = value;
	}

	const std::shared_ptr<Video>& Layer::GetVideoItem()
	{
		return references.Video;
	}

	const std::shared_ptr<Audio>& Layer::GetAudioItem()
	{
		return references.Audio;
	}

	const std::shared_ptr<Composition>& Layer::GetCompItem()
	{
		return references.Composition;
	}

	const Video* Layer::GetVideoItem() const
	{
		return references.Video.get();
	}

	const Audio* Layer::GetAudioItem() const
	{
		return references.Audio.get();
	}

	const Composition* Layer::GetCompItem() const
	{
		return references.Composition.get();
	}

	void Layer::SetItem(const std::shared_ptr<Video>& value)
	{
		references.Video = value;
	}

	void Layer::SetItem(const std::shared_ptr<Audio>& value)
	{
		references.Audio = value;
	}

	void Layer::SetItem(const std::shared_ptr<Composition>& value)
	{
		references.Composition = value;
	}

	const std::shared_ptr<Layer>& Layer::GetRefParentLayer()
	{
		return references.ParentLayer;
	}

	const Layer* Layer::GetRefParentLayer() const
	{
		return references.ParentLayer.get();
	}

	void Layer::SetRefParentLayer(const std::shared_ptr<Layer>& value)
	{
		references.ParentLayer = value;
	}

	std::optional<frame_t> Layer::FindMarkerFrame(std::string_view markerName) const
	{
		auto found = FindIfOrNull(Markers, [&](const auto& m) { return m->Name == markerName; });
		return (found != nullptr) ? (*found)->Frame : std::optional<frame_t> {};
	}

	Scene* Layer::GetParentScene()
	{
		assert(parentComposition != nullptr);
		return parentComposition->GetParentScene();
	}

	const Scene* Layer::GetParentScene() const
	{
		assert(parentComposition != nullptr);
		return parentComposition->GetParentScene();
	}

	Composition* Layer::GetParentComposition()
	{
		return parentComposition;
	}

	const Composition* Layer::GetParentComposition() const
	{
		return parentComposition;
	}

	Scene* Composition::GetParentScene() const
	{
		return parentScene;
	}

	bool Composition::IsRootComposition() const
	{
		return this == parentScene->RootComposition.get();
	}

	std::shared_ptr<Layer> Composition::FindLayer(std::string_view name)
	{
		auto result = std::find_if(layers.begin(), layers.end(), [&](auto& layer) { return layer->GetName() == name; });
		return (result != layers.end()) ? *result : nullptr;
	}

	std::shared_ptr<const Layer> Composition::FindLayer(std::string_view name) const
	{
		return const_cast<Composition*>(this)->FindLayer(name);
	}

	Composition* Scene::GetRootComposition()
	{
		return RootComposition.get();
	}

	const Composition* Scene::GetRootComposition() const
	{
		return RootComposition.get();
	}

	std::shared_ptr<Layer> Scene::FindLayer(std::string_view name)
	{
		const std::shared_ptr<Layer>& rootFoundLayer = RootComposition->FindLayer(name);
		if (rootFoundLayer != nullptr)
			return rootFoundLayer;

		for (i32 i = static_cast<i32>(Compositions.size()) - 1; i >= 0; i--)
		{
			const std::shared_ptr<Layer>& layer = Compositions[i]->FindLayer(name);
			if (layer != nullptr)
				return layer;
		}

		return nullptr;
	}

	std::shared_ptr<const Layer> Scene::FindLayer(std::string_view name) const
	{
		return const_cast<Scene*>(this)->FindLayer(name);
	}

	int Scene::FindLayerIndex(Composition& comp, std::string_view name) const
	{
		for (int i = static_cast<int>(comp.GetLayers().size()) - 1; i >= 0; i--)
		{
			if (comp.GetLayers()[i]->GetName() == name)
				return i;
		}

		return -1;
	}

	void Scene::UpdateParentPointers()
	{
		ForEachComp([&](auto& comp)
		{
			comp->parentScene = this;

			for (auto& layer : comp->GetLayers())
				layer->parentComposition = comp.get();
		});
	}

	void Scene::UpdateCompNamesAfterLayerItems()
	{
		RootComposition->SetName(Composition::rootCompositionName);
		UpdateCompNamesAfterLayerItems(RootComposition);

		for (auto& comp : Compositions)
			UpdateCompNamesAfterLayerItems(comp);
	}

	void Scene::UpdateCompNamesAfterLayerItems(std::shared_ptr<Composition>& comp)
	{
		for (auto& layer : comp->GetLayers())
		{
			if (layer->ItemType == ItemType::Composition)
			{
				if (auto compItem = layer->GetCompItem().get(); compItem != nullptr)
					compItem->SetName(layer->GetName());
			}
		}
	}

	void Scene::LinkPostRead()
	{
		assert(RootComposition != nullptr);
		ForEachComp([&](auto& comp) { LinkCompItems(*comp); });
	}

	void Scene::LinkCompItems(Composition& comp)
	{
		auto findSetLayerItem = [](auto& layer, auto& itemCollection)
		{
			auto foundItem = FindIfOrNull(itemCollection, [&](const auto& e) { return e->filePosition == layer.itemFileOffset; });
			if (foundItem != nullptr)
				layer.SetItem(*foundItem);
		};

		for (auto& layer : comp.GetLayers())
		{
			if (layer->itemFileOffset != FileAddr::NullPtr)
			{
				if (layer->ItemType == ItemType::Video)
					findSetLayerItem(*layer, Videos);
				else if (layer->ItemType == ItemType::Audio)
					findSetLayerItem(*layer, Audios);
				else if (layer->ItemType == ItemType::Composition)
					findSetLayerItem(*layer, Compositions);
			}

			if (layer->parentFileOffset != FileAddr::NullPtr)
				FindSetLayerRefParentLayer(*layer);
		}
	}

	void Scene::FindSetLayerRefParentLayer(Layer& layer)
	{
		for (auto& otherComp : Compositions)
		{
			for (auto& otherLayer : otherComp->GetLayers())
			{
				if (otherLayer->filePosition == layer.parentFileOffset)
				{
					layer.references.ParentLayer = otherLayer;
					return;
				}
			}
		}
	}

	void AetSet::ClearSpriteCache()
	{
		for (auto& scene : scenes)
		{
			for (auto& video : scene->Videos)
			{
				for (auto& source : video->Sources)
					source.SpriteCache = nullptr;
			}
		}
	}
}

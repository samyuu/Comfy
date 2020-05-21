#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Graphics/Auth2D/Aet/AetSet.h"

namespace Comfy::Studio::Editor
{
	enum class AetItemType
	{
		None,
		AetSet,
		Scene,
		Composition,
		Layer,
		Video,
	};

	union AetItemPtrUnion
	{
		void* Void;
		Graphics::Aet::AetSet* AetSet;
		Graphics::Aet::Scene* Scene;
		Graphics::Aet::Composition* Composition;
		Graphics::Aet::Layer* Layer;
		Graphics::Aet::Video* Video;
	};

	union AetItemReferencePtrUnion
	{
		const std::shared_ptr<void*>* VoidRef;
		const std::shared_ptr<Graphics::Aet::AetSet>* AetSetRef;
		const std::shared_ptr<Graphics::Aet::Scene>* SceneRef;
		const std::shared_ptr<Graphics::Aet::Composition>* CompositionRef;
		const std::shared_ptr<Graphics::Aet::Layer>* LayerRef;
		const std::shared_ptr<Graphics::Aet::Video>* VideoRef;
	};

	struct AetItemTypePtr
	{
	public:
		template <typename T>
		void SetItem(const std::shared_ptr<T>& value);

		inline AetItemType Type() const { return type; }
		inline void Reset() { type = AetItemType::None; Ptrs.Void = nullptr; refPtrs.VoidRef = nullptr; }

	public:
		inline const std::shared_ptr<Graphics::Aet::AetSet>& GetAetSetRef() const { return *refPtrs.AetSetRef; }
		inline const std::shared_ptr<Graphics::Aet::Scene>& GetSceneRef() const { return *refPtrs.SceneRef; }
		inline const std::shared_ptr<Graphics::Aet::Composition>& GetCompositionRef() const { return *refPtrs.CompositionRef; }
		inline const std::shared_ptr<Graphics::Aet::Layer>& GetLayerRef() const { return *refPtrs.LayerRef; }
		inline const std::shared_ptr<Graphics::Aet::Video>& GetVideoRef() const { return *refPtrs.VideoRef; }

		inline bool IsNull() const { return Ptrs.Void == nullptr; }
		inline Graphics::Aet::Scene* GetItemParentScene() const;

	public:
		AetItemPtrUnion Ptrs;

	private:
		AetItemType type;
		AetItemReferencePtrUnion refPtrs;
	};

	template <typename T>
	void AetItemTypePtr::SetItem(const std::shared_ptr<T>& value)
	{
		if constexpr (std::is_same<T, Graphics::Aet::AetSet>::value)
			type = AetItemType::AetSet;
		else if constexpr (std::is_same<T, Graphics::Aet::Scene>::value)
			type = AetItemType::Scene;
		else if constexpr (std::is_same<T, Graphics::Aet::Composition>::value)
			type = AetItemType::Composition;
		else if constexpr (std::is_same<T, Graphics::Aet::Layer>::value)
			type = AetItemType::Layer;
		else if constexpr (std::is_same<T, Graphics::Aet::Video>::value)
			type = AetItemType::Video;
		else
			static_assert(false, "Invalid Type T");

		Ptrs.Void = reinterpret_cast<void*>(value.get());
		refPtrs.VoidRef = reinterpret_cast<const std::shared_ptr<void*>*>(&value);
	}

	Graphics::Aet::Scene* AetItemTypePtr::GetItemParentScene() const
	{
		if (IsNull())
			return nullptr;

		switch (type)
		{
		case AetItemType::None:
			return nullptr;
		case AetItemType::AetSet:
			return nullptr;
		case AetItemType::Scene:
			return Ptrs.Scene;
		case AetItemType::Composition:
			assert(Ptrs.Composition->GetParentScene() != nullptr);
			return Ptrs.Composition->GetParentScene();
		case AetItemType::Layer:
			assert(Ptrs.Layer->GetParentScene() != nullptr);
			return Ptrs.Layer->GetParentScene();
		case AetItemType::Video:
			return nullptr;
		}
		return nullptr;
	}
}

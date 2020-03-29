#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Graphics/Auth2D/Aet/AetSet.h"

namespace Comfy::Editor
{
	enum class AetItemType
	{
		None,
		AetSet,
		Aet,
		Composition,
		Layer,
		Surface,
	};

	union AetItemPtrUnion
	{
		void* VoidPointer;

		Graphics::AetSet* AetSet;
		Graphics::Aet* Aet;
		Graphics::AetComposition* Composition;
		Graphics::AetLayer* Layer;
		Graphics::AetSurface* Surface;
	};

	union AetItemReferencePtrUnion
	{
		const RefPtr<void*>* VoidReference;
		const RefPtr<Graphics::AetSet>* AetSetRef;
		const RefPtr<Graphics::Aet>* AetRef;
		const RefPtr<Graphics::AetComposition>* CompositionRef;
		const RefPtr<Graphics::AetLayer>* LayerRef;
		const RefPtr<Graphics::AetSurface>* SurfaceRef;
	};

	struct AetItemTypePtr
	{
	public:
		template <typename T>
		inline void SetItem(const RefPtr<T>& value);

		inline AetItemType Type() const { return type; };
		inline void Reset() { type = AetItemType::None; Ptrs.VoidPointer = nullptr; refPtrs.VoidReference = nullptr; };

	public:
		inline const RefPtr<Graphics::AetSet>& GetAetSetRef() const { return *refPtrs.AetSetRef; };
		inline const RefPtr<Graphics::Aet>& GetAetRef() const { return *refPtrs.AetRef; };
		inline const RefPtr<Graphics::AetComposition>& GetAetCompositionRef() const { return *refPtrs.CompositionRef; };
		inline const RefPtr<Graphics::AetLayer>& GetLayerRef() const { return *refPtrs.LayerRef; };
		inline const RefPtr<Graphics::AetSurface>& GetSurfaceRef() const { return *refPtrs.SurfaceRef; };

		inline bool IsNull() const { return Ptrs.VoidPointer == nullptr; };
		inline Graphics::Aet* GetItemParentAet() const;

	public:
		AetItemPtrUnion Ptrs;

	private:
		AetItemType type;
		AetItemReferencePtrUnion refPtrs;
	};

	template<class T>
	inline void AetItemTypePtr::SetItem(const RefPtr<T>& value)
	{
		if constexpr (std::is_same<T, Graphics::AetSet>::value)
		{
			type = AetItemType::AetSet;
			Ptrs.AetSet = value.get();
			refPtrs.AetSetRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::Aet>::value)
		{
			type = AetItemType::Aet;
			Ptrs.Aet = value.get();
			refPtrs.AetRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::AetComposition>::value)
		{
			type = AetItemType::Composition;
			Ptrs.Composition = value.get();
			refPtrs.CompositionRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::AetLayer>::value)
		{
			type = AetItemType::Layer;
			Ptrs.Layer = value.get();
			refPtrs.LayerRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::AetSurface>::value)
		{
			type = AetItemType::Surface;
			Ptrs.Surface = value.get();
			refPtrs.SurfaceRef = &value;
		}
		else
		{
			static_assert(false, "Invalid Type T");
		}
	}

	inline Graphics::Aet* AetItemTypePtr::GetItemParentAet() const
	{
		if (IsNull())
			return nullptr;

		switch (type)
		{
		case AetItemType::None:
			return nullptr;
		case AetItemType::AetSet:
			return nullptr;
		case AetItemType::Aet:
			return Ptrs.Aet;
		case AetItemType::Composition:
			assert(Ptrs.Composition->GetParentAet() != nullptr);
			return Ptrs.Composition->GetParentAet();
		case AetItemType::Layer:
			assert(Ptrs.Layer->GetParentAet() != nullptr);
			return Ptrs.Layer->GetParentAet();
		case AetItemType::Surface:
			return nullptr;
		}
		return nullptr;
	}
}

#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "Graphics/Auth2D/AetSet.h"

namespace Editor
{
	enum class AetItemType
	{
		None,
		AetSet,
		Aet,
		AetLayer,
		AetObj,
		AetRegion,
	};

	union AetItemPtrUnion
	{
		void* VoidPointer;

		Graphics::AetSet* AetSet;
		Graphics::Aet* Aet;
		Graphics::AetLayer* AetLayer;
		Graphics::AetObj* AetObj;
		Graphics::AetRegion* AetRegion;
	};

	union AetItemReferencePtrUnion
	{
		const RefPtr<void*>* VoidReference;
		const RefPtr<Graphics::AetSet>* AetSetRef;
		const RefPtr<Graphics::Aet>* AetRef;
		const RefPtr<Graphics::AetLayer>* AetLayerRef;
		const RefPtr<Graphics::AetObj>* AetObjRef;
		const RefPtr<Graphics::AetRegion>* AetRegionRef;
	};

	struct AetItemTypePtr
	{
	public:
		template <class T>
		inline void SetItem(const RefPtr<T>& value);

		inline AetItemType Type() const { return type; };
		inline void Reset() { type = AetItemType::None; Ptrs.VoidPointer = nullptr; refPtrs.VoidReference = nullptr; };

	public:
		inline const RefPtr<Graphics::AetSet>& GetAetSetRef() const { return *refPtrs.AetSetRef; };
		inline const RefPtr<Graphics::Aet>& GetAetRef() const { return *refPtrs.AetRef; };
		inline const RefPtr<Graphics::AetLayer>& GetAetLayerRef() const { return *refPtrs.AetLayerRef; };
		inline const RefPtr<Graphics::AetObj>& GetAetObjRef() const { return *refPtrs.AetObjRef; };
		inline const RefPtr<Graphics::AetRegion>& GetAetRegionRef() const { return *refPtrs.AetRegionRef; };

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
		else if constexpr (std::is_same<T, Graphics::AetLayer>::value)
		{
			type = AetItemType::AetLayer;
			Ptrs.AetLayer = value.get();
			refPtrs.AetLayerRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::AetObj>::value)
		{
			type = AetItemType::AetObj;
			Ptrs.AetObj = value.get();
			refPtrs.AetObjRef = &value;
		}
		else if constexpr (std::is_same<T, Graphics::AetRegion>::value)
		{
			type = AetItemType::AetRegion;
			Ptrs.AetRegion = value.get();
			refPtrs.AetRegionRef = &value;
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
		case AetItemType::AetLayer:
			assert(Ptrs.AetLayer->GetParentAet() != nullptr);
			return Ptrs.AetLayer->GetParentAet();
		case AetItemType::AetObj:
			assert(Ptrs.AetObj->GetParentAet() != nullptr);
			return Ptrs.AetObj->GetParentAet();
		case AetItemType::AetRegion:
			return nullptr;
		}
		return nullptr;
	}
}
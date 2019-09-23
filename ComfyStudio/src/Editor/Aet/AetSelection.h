#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

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

		AetSet* AetSet;
		Aet* Aet;
		AetLayer* AetLayer;
		AetObj* AetObj;
		AetRegion* AetRegion;
	};

	union AetItemReferencePtrUnion
	{
		const RefPtr<void*>* VoidReference;
		const RefPtr<AetSet>* AetSetRef;
		const RefPtr<Aet>* AetRef;
		const RefPtr<AetLayer>* AetLayerRef;
		const RefPtr<AetObj>* AetObjRef;
		const RefPtr<AetRegion>* AetRegionRef;
	};

	struct AetItemTypePtr
	{
	public:
		template <class T>
		inline void SetItem(const RefPtr<T>& value);

		inline AetItemType Type() const { return type; };
		inline void Reset() { type = AetItemType::None; Ptrs.VoidPointer = nullptr; refPtrs.VoidReference = nullptr; };

	public:
		inline const RefPtr<AetSet>& GetAetSetRef() const { return *refPtrs.AetSetRef; };
		inline const RefPtr<Aet>& GetAetRef() const { return *refPtrs.AetRef; };
		inline const RefPtr<AetLayer>& GetAetLayerRef() const { return *refPtrs.AetLayerRef; };
		inline const RefPtr<AetObj>& GetAetObjRef() const { return *refPtrs.AetObjRef; };
		inline const RefPtr<AetRegion>& GetAetRegionRef() const { return *refPtrs.AetRegionRef; };

		inline bool IsNull() const { return Ptrs.VoidPointer == nullptr; };
		inline Aet* GetItemParentAet() const;

	public:
		AetItemPtrUnion Ptrs;

	private:
		AetItemType type;
		AetItemReferencePtrUnion refPtrs;
	};

	template<class T>
	inline void AetItemTypePtr::SetItem(const RefPtr<T>& value)
	{
		if constexpr (std::is_same<T, AetSet>::value)
		{
			type = AetItemType::AetSet;
			Ptrs.AetSet = value.get();
			refPtrs.AetSetRef = &value;
		}
		else if constexpr (std::is_same<T, Aet>::value)
		{
			type = AetItemType::Aet;
			Ptrs.Aet = value.get();
			refPtrs.AetRef = &value;
		}
		else if constexpr (std::is_same<T, AetLayer>::value)
		{
			type = AetItemType::AetLayer;
			Ptrs.AetLayer = value.get();
			refPtrs.AetLayerRef = &value;
		}
		else if constexpr (std::is_same<T, AetObj>::value)
		{
			type = AetItemType::AetObj;
			Ptrs.AetObj = value.get();
			refPtrs.AetObjRef = &value;
		}
		else if constexpr (std::is_same<T, AetRegion>::value)
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

	inline Aet* AetItemTypePtr::GetItemParentAet() const
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
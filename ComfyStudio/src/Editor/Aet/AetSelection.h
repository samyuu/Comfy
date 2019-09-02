#pragma once
#include "Editor/Core/IEditorComponent.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	enum class AetSelectionType
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

	struct AetItemReferencePtrUnion
	{
		union
		{
			const RefPtr<void*>* VoidReference;
			const RefPtr<AetSet>* AetSetRef;
			const RefPtr<Aet>* AetRef;
			const RefPtr<AetLayer>* AetLayerRef;
			const RefPtr<AetObj>* AetObjRef;
			const RefPtr<AetRegion>* AetRegionRef;
		};
	};

	struct AetItemTypePtr
	{
	public:
		inline void SetItem(const RefPtr<AetSet>& value) { type = AetSelectionType::AetSet; Ptrs.AetSet = value.get(); refPtrs.AetSetRef = &value; }
		inline void SetItem(const RefPtr<Aet>& value) { type = AetSelectionType::Aet; Ptrs.Aet = value.get(); refPtrs.AetRef = &value; }
		inline void SetItem(const RefPtr<AetLayer>& value) { type = AetSelectionType::AetLayer; Ptrs.AetLayer = value.get(); refPtrs.AetLayerRef = &value; }
		inline void SetItem(const RefPtr<AetObj>& value) { type = AetSelectionType::AetObj; Ptrs.AetObj = value.get(); refPtrs.AetObjRef = &value; }
		inline void SetItem(const RefPtr<AetRegion>& value) { type = AetSelectionType::AetRegion; Ptrs.AetRegion = value.get(); refPtrs.AetRegionRef = &value; }

		inline AetSelectionType Type() const { return type; };
		inline void Reset() { type = AetSelectionType::None; Ptrs.VoidPointer = nullptr; refPtrs.VoidReference = nullptr; };

	public:
		inline const RefPtr<AetSet>& GetAetSetRef() const { return *refPtrs.AetSetRef; };
		inline const RefPtr<Aet>& GetAetRef() const { return *refPtrs.AetRef; };
		inline const RefPtr<AetLayer>& GetAetLayerRef() const { return *refPtrs.AetLayerRef; };
		inline const RefPtr<AetObj>& GetAetObjRef() const { return *refPtrs.AetObjRef; };
		inline const RefPtr<AetRegion>& GetAetRegionRef() const { return *refPtrs.AetRegionRef; };

		inline bool IsNull() { return Ptrs.VoidPointer == nullptr; };

	public:
		AetItemPtrUnion Ptrs;

	private:
		AetSelectionType type;
		AetItemReferencePtrUnion refPtrs;
	};
}
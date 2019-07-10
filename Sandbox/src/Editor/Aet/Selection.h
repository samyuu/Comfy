#pragma once
#include "Editor/IEditorComponent.h"
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

	struct AetItemTypePtr
	{
	public:
		union
		{
			void* VoidPointer;

			AetSet* AetSet;
			Aet* Aet;
			AetLayer* AetLayer;
			AetObj* AetObj;
			AetRegion* AetRegion;
		};

		#define Inline_SetItemMethod(typeName) inline void SetItem(FileSystem::##typeName* value) { type = AetSelectionType::##typeName; typeName = value; }
		// --------------------------
		Inline_SetItemMethod(AetSet);
		Inline_SetItemMethod(Aet);
		Inline_SetItemMethod(AetLayer);
		Inline_SetItemMethod(AetObj);
		Inline_SetItemMethod(AetRegion);
		// --------------------------
		#undef Inline_SetItemMethod

		inline AetSelectionType Type() const	{ return type; };
		inline void Reset()						{ type = AetSelectionType::None; VoidPointer = nullptr; };

	private:
		AetSelectionType type;
	};
}
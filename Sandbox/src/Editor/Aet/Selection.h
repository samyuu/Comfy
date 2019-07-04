#pragma once
#include "Editor/IEditorComponent.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	enum class AetSelectionType
	{
		None,
		AetObj,
		AetLayer,
		AetLyo,
		AetRegion,
	};

	struct AetItemTypePtr
	{
		AetSelectionType Type;
		union
		{
			void* ItemPtr;
			AetObj* AetObj;
			AetLyo* AetLyo;
			AetLayer* AetLayer;
			AetRegion* AetRegion;
		};
	};
}
#pragma once
#include "Selection.h"
#include "FileSystem/Format/AetSet.h"

namespace Editor
{
	using namespace FileSystem;

	class AetTreeView
	{
	public:
		AetTreeView(AetLyo** activeAetLyo, AetItemTypePtr* selected, AetItemTypePtr* hovered, AetItemTypePtr* lastHovered);
		~AetTreeView();

		void Initialize();
		bool DrawGui(AetSet* aetSet);

		inline void SetSelectedItem(AetLyo* aetLyo, AetObj* value) { *activeAetLyo = aetLyo; *selected = { AetSelectionType::AetObj, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetLyo* value) { *activeAetLyo = aetLyo; *selected = { AetSelectionType::AetLyo, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetLayer* value) { *activeAetLyo = aetLyo; *selected = { AetSelectionType::AetLayer, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetRegion* value) {* activeAetLyo = aetLyo; *selected = { AetSelectionType::AetRegion, value }; };
		inline void ResetSelectedItem() { *activeAetLyo = nullptr; *selected = { AetSelectionType::None, nullptr }; };

	private:
		const char* aetLayerContextMenuID = "AetLayerContextMenu";
		const char* addAetObjPopupID = "Add new AetObj";

		int newObjTypeIndex = AetObjType_Pic;
		char newObjNameBuffer[255];

		AetLyo** activeAetLyo;
		AetItemTypePtr *selected, *lastHovered, *hovered;

		void DrawTreeViewLyo(AetLyo& aetLyo);
		void DrawTreeViewLayer(AetLyo& aetLyo, AetLayer& aetLayer);
		void DrawTreeViewRegion(AetLyo& aetLyo, AetRegion& region, int32_t index);
	};
}
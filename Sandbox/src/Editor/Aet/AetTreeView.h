#pragma once
#include "Selection.h"
#include "FileSystem/Format/AetSet.h"
#include <vector>

namespace Editor
{
	using namespace FileSystem;

	class AetTreeView
	{
	public:
		AetTreeView();
		~AetTreeView();

		void Initialize();
		bool DrawGui(AetSet* aetSet);

		inline AetLyo* GetActiveAetLyo() const { return activeAetLyo; };
		inline AetItemTypePtr GetSelected() const { return selected; };

		inline void SetSelectedItem(AetLyo* aetLyo, AetObj* value) { activeAetLyo = aetLyo; selected = { AetSelectionType::AetObj, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetLyo* value) { activeAetLyo = aetLyo; selected = { AetSelectionType::AetLyo, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetLayer* value) { activeAetLyo = aetLyo; selected = { AetSelectionType::AetLayer, value }; };
		inline void SetSelectedItem(AetLyo* aetLyo, AetRegion* value) { activeAetLyo = aetLyo; selected = { AetSelectionType::AetRegion, value }; };
		inline void ResetSelectedItem() { activeAetLyo = nullptr; selected = { AetSelectionType::None, nullptr }; };

	private:
		const char* aetLayerContextMenuID = "AetLayerContextMenu";
		const char* addAetObjPopupID = "Add new AetObj";

		int newObjTypeIndex = AetObjType_Pic;
		char newObjNameBuffer[255];

		AetLyo* activeAetLyo;
		AetItemTypePtr selected, lastHovered, hovered;

		std::vector<bool> openLayers;

		void DrawTreeViewLyo(AetLyo& aetLyo);
		void DrawTreeViewLayer(AetLyo& aetLyo, AetLayer& aetLayer);
		void DrawTreeViewObj(AetLyo& aetLyo, AetObj& aetObj);
		void DrawTreeViewRegion(AetLyo& aetLyo, AetRegion& region, int32_t index);
	
		bool AddAetObjContextMenu(AetLayer& aetLayer);
		void AddAetObjPopup(AetLayer& aetLayer);

		const char* GetTypeIcon(AetObjType type) const;
	};
}
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

		inline Aet* GetActiveAet() const { return activeAet; };
		inline AetItemTypePtr GetSelected() const { return selected; };

		inline void SetSelectedItem(AetSet* value)				{ activeAet = nullptr; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, Aet* value)		{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, AetObj* value)	{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, AetLayer* value)	{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, AetRegion* value)	{ activeAet = aet; selected.SetItem(value); };
		inline void ResetSelectedItem()							{ activeAet = nullptr; selected.Reset(); };

	private:
		const char* aetLayerContextMenuID = "AetLayerContextMenu";
		const char* addAetObjPopupID = "Add new AetObj";

		int newObjTypeIndex = static_cast<int>(AetObjType::Pic);
		char objNameBuffer[255];
		char newObjNameBuffer[255];
		char regionNameBuffer[255];

		Aet* activeAet;
		AetItemTypePtr selected, lastHovered, hovered;

		std::vector<std::vector<bool>> openLayers;

		void DrawTreeViewAet(Aet& aet);
		void DrawTreeViewLayer(Aet& aet, AetLayer& aetLayer);
		void DrawTreeViewObj(Aet& aet, AetObj& aetObj);
		void DrawTreeViewRegion(Aet& aet, AetRegion& region, int32_t index);
	
		bool AddAetObjContextMenu(AetLayer& aetLayer);
		void AddAetObjPopup(AetLayer& aetLayer);
	};
}
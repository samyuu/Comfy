#pragma once
#include "Selection.h"
#include "AetDialogs.h"
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
		const char* addAetObjPopupID = "Add new AetObj";

		char objNameBuffer[255];
		char regionNameBuffer[255];

		AddAetObjDialog addAetObjDialog;

		Aet* activeAet;
		AetItemTypePtr selected, lastHovered, hovered;

		void DrawTreeViewBackground();
		void DrawTreeViewAet(Aet& aet);
		void DrawTreeViewLayer(Aet& aet, AetLayer& aetLayer);
		void DrawTreeViewObj(Aet& aet, AetObj& aetObj);
		void DrawTreeViewRegion(Aet& aet, AetRegion& region, int32_t index);
	
		bool DrawAetLayerContextMenu(AetLayer& aetLayer);
		bool DrawAetObjContextMenu(AetObj& aetObj);
	};
}
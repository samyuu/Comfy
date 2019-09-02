#pragma once
#include "Editor/Aet/AetSelection.h"
#include "AetDialogs.h"
#include "FileSystem/Format/AetSet.h"
#include "Core/CoreTypes.h"
#include "ImGui/Gui.h"

namespace Editor
{
	using namespace FileSystem;

	class AetTreeView
	{
	public:
		AetTreeView();
		~AetTreeView();

		void Initialize();
		bool DrawGui(const RefPtr<AetSet>& aetSet);

		inline Aet* GetActiveAet() const { return activeAet; };
		inline AetItemTypePtr GetSelected() const { return selected; };

		inline void SetSelectedItem(const RefPtr<AetSet>& value)				{ activeAet = nullptr; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, const RefPtr<Aet>& value)			{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, const RefPtr<AetObj>& value)		{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, const RefPtr<AetLayer>& value)	{ activeAet = aet; selected.SetItem(value); };
		inline void SetSelectedItem(Aet* aet, const RefPtr<AetRegion>& value)	{ activeAet = aet; selected.SetItem(value); };
		inline void ResetSelectedItem()											{ activeAet = nullptr; selected.Reset(); };

	private:
		const char* addAetObjPopupID = "Add new AetObj";

		char objNameBuffer[255];
		char regionNameBuffer[255];

		AddAetObjDialog addAetObjDialog;

		Aet* activeAet;
		AetItemTypePtr selected, lastHovered, hovered;

		void DrawTreeViewBackground();
		void DrawTreeViewAet(const RefPtr<Aet>& aet);
		void DrawTreeViewLayer(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer);
		void DrawTreeViewObj(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj);
		void DrawTreeViewRegion(const RefPtr<Aet>& aet, const RefPtr<AetRegion>& region, int32_t index);
	
		bool DrawAetLayerContextMenu(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer);
		bool DrawAetObjContextMenu(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj);

	private:
		const char* GetDebugObjectName();
	};
}
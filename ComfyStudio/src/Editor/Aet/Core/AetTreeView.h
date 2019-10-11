#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/AetIcons.h"
#include "AetDialogs.h"
#include "FileSystem/Format/AetSet.h"
#include "Core/CoreTypes.h"
#include "ImGui/Gui.h"
#include <stack>

namespace Editor
{
	using namespace FileSystem;

	class AetTreeView : public IMutatingEditorComponent
	{
	public:
		AetTreeView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem);
		~AetTreeView();

		void Initialize();
		bool DrawGui(const RefPtr<AetSet>& aetSet);

		template <class T>
		inline void SetSelectedItems(const RefPtr<T>& value) 
		{ 
			selectedAetItem->SetItem(value);
			cameraSelectedAetItem->SetItem(value);
		};

		inline void SetSelectedItems(const RefPtr<AetObj>& selectedObj, const RefPtr<AetLayer>& visibleLayer)
		{
			selectedAetItem->SetItem(selectedObj);
			cameraSelectedAetItem->SetItem(visibleLayer);
		};

		inline void ResetSelectedItems() 
		{ 
			selectedAetItem->Reset();
			cameraSelectedAetItem->Reset();
		};

	private:
		static constexpr const char* textureMaskIndicator = "  ( " ICON_FA_LINK " )";
		static constexpr const char* addAetObjPopupID = "Add new AetObj";
		static constexpr float layerPreviewTooltipHoverDelay = 0.8f;
		static constexpr int layerPreviewMaxConunt = 10;

		char nodeNameFormatBuffer[512];
		
		float scrollTargetCenterRatio = 0.15f;
		AddAetObjDialog addAetObjDialog;

		std::stack<float> scrollPositionStack;

		struct
		{
			AetItemTypePtr* selectedAetItem;
			AetItemTypePtr* cameraSelectedAetItem;
			AetItemTypePtr hoveredAetItem, lastHoveredAetItem;
		};

		ImGuiWindow* treeViewWindow = nullptr;

		void UpdateScrollButtonInput();

		void DrawTreeViewBackground();
		
		// TODO: These should probably be called DrawTreeNode{Name}
		void DrawTreeViewAetSet(const RefPtr<AetSet>& aetSet);
		
		void DrawTreeViewAet(const RefPtr<Aet>& aet);
		void DrawTreeViewLayer(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, bool isRoot);

		void DrawTreeViewObj(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj);
		void DrawTreeViewObjCameraSelectableButton(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj);
		void DrawTreeViewObjActivityButton(const RefPtr<AetObj>& aetObj);

		void DrawTreeViewRegion(const RefPtr<Aet>& aet, const RefPtr<AetRegion>& region, int32_t index);

		bool DrawAetLayerContextMenu(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, bool isRoot);
		bool DrawAetObjContextMenu(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj);

		void DrawAetLayerPreviewTooltip(const RefPtr<AetLayer>& aetLayer);
		void DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const;

	private:
		const char* FormatRegionNodeName(const RefPtr<AetRegion>& region, int32_t index);

	private:
		void ScrollToGuiData(GuiTempData& guiData);

	private:
		const char* GetDebugObjectName();
	};
}
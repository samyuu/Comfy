#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/AetIcons.h"
#include "AetDialogs.h"
#include "Graphics/Auth2D/AetSet.h"
#include "Core/CoreTypes.h"
#include "ImGui/Gui.h"
#include <stack>

namespace Editor
{
	class AetTreeView : public IMutatingEditorComponent
	{
	public:
		AetTreeView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem);
		AetTreeView(const AetTreeView&) = delete;
		AetTreeView& operator= (AetTreeView&) = delete;
		~AetTreeView();

		void Initialize();
		bool DrawGui(const RefPtr<Graphics::AetSet>& aetSet);

		template <class T>
		inline void SetSelectedItems(const RefPtr<T>& value) 
		{ 
			selectedAetItem->SetItem(value);
			cameraSelectedAetItem->SetItem(value);
		};

		inline void SetSelectedItems(const RefPtr<Graphics::AetObj>& selectedObj, const RefPtr<Graphics::AetLayer>& visibleLayer)
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
		static constexpr float layerPreviewTooltipHoverDelay = 0.8f;
		static constexpr int layerPreviewMaxConunt = 10;

		char nodeNameFormatBuffer[512];
		
		float scrollTargetCenterRatio = 0.15f;

		// TODO: Remove deprecated components
		// AddAetObjDialog addAetObjDialog;

		std::stack<float> scrollPositionStack;

		// NOTE: To be used, filled and cleared by the layer usage context menu
		std::vector<RefPtr<Graphics::AetObj>*> layerUsagesBuffer;

		struct
		{
			AetItemTypePtr* selectedAetItem;
			AetItemTypePtr* cameraSelectedAetItem;
			AetItemTypePtr hoveredAetItem, lastHoveredAetItem;
		};

		ImGuiWindow* treeViewWindow = nullptr;

		void UpdateScrollButtonInput();

		void DrawTreeViewBackground();
		
		// TODO: These should probably be named DrawTreeNode{Name}
		void DrawTreeViewAetSet(const RefPtr<Graphics::AetSet>& aetSet);
		
		void DrawTreeViewAet(const RefPtr<Graphics::Aet>& aet);
		void DrawTreeViewLayer(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, bool isRoot);

		void DrawTreeViewObj(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawTreeViewObjCameraSelectableButton(const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawTreeViewObjActivityButton(const RefPtr<Graphics::AetObj>& aetObj);

		void DrawTreeViewRegion(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetRegion>& region, int32_t index);

		bool DrawAetLayerContextMenu(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, bool isRoot);
		bool DrawAetObjContextMenu(const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);

		void DrawAetLayerPreviewTooltip(const RefPtr<Graphics::AetLayer>& aetLayer);
		void DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const;

	private:
		const char* FormatRegionNodeName(const RefPtr<Graphics::AetRegion>& region, int32_t index);

	private:
		void ScrollToGuiData(GuiExtraData& guiData);

	private:
		const char* GetDebugObjectName();
	};
}
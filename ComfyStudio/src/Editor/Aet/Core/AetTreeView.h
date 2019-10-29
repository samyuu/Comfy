#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/AetIcons.h"
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

	private:
		void DrawTreeViewBackground();
		
		void DrawTreeNodeAetSet(const RefPtr<Graphics::AetSet>& aetSet);
		
		void DrawTreeNodeAet(const RefPtr<Graphics::Aet>& aet);
		void DrawTreeNodeLayer(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, bool isRoot);

		void DrawTreeNodeObj(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawTreeNodeObjCameraSelectableButton(const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);
		void DrawTreeNodeObjActivityButton(const RefPtr<Graphics::AetObj>& aetObj);

		void DrawTreeNodeRegion(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetRegion>& region, int32_t index);

		bool DrawAetLayerContextMenu(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetLayer>& aetLayer, bool isRoot);
		bool DrawAetObjContextMenu(const RefPtr<Graphics::AetLayer>& aetLayer, const RefPtr<Graphics::AetObj>& aetObj);

		void DrawAetLayerPreviewTooltip(const RefPtr<Graphics::AetLayer>& aetLayer);
		void DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const;

	private:
		const char* FormatRegionNodeName(const RefPtr<Graphics::AetRegion>& region, int32_t index);

	private:
		void UpdateScrollButtonInput();
		void ScrollToGuiData(GuiExtraData& guiData);

	private:
		const char* GetDebugObjectName();
	};
}
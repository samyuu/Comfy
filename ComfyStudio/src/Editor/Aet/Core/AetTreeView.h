#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Aet/AetIcons.h"
#include "Graphics/Auth2D/Aet/AetSet.h"
#include "ImGui/Gui.h"
#include <stack>

namespace Comfy::Editor
{
	class AetTreeView : public IMutatingEditorComponent
	{
	public:
		AetTreeView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem);
		~AetTreeView();

	public:
		void Initialize();
		bool DrawGui(const RefPtr<Graphics::AetSet>& aetSet);

		template <typename T>
		inline void SetSelectedItems(const RefPtr<T>& value) 
		{ 
			selectedAetItem->SetItem(value);
			cameraSelectedAetItem->SetItem(value);
		};

		inline void SetSelectedItems(const RefPtr<Graphics::AetLayer>& selectedLayer, const RefPtr<Graphics::AetComposition>& visibleComp)
		{
			selectedAetItem->SetItem(selectedLayer);
			cameraSelectedAetItem->SetItem(visibleComp);
		};

		inline void ResetSelectedItems() 
		{ 
			selectedAetItem->Reset();
			cameraSelectedAetItem->Reset();
		};

	private:
		static constexpr const char* textureMaskIndicator = "  ( " ICON_FA_LINK " )";
		static constexpr float compPreviewTooltipHoverDelay = 0.8f;
		static constexpr int compPreviewMaxConunt = 10;

		char nodeNameFormatBuffer[512];
		
		float scrollTargetCenterRatio = 0.15f;

		std::stack<float> scrollPositionStack;

		// NOTE: To be used, filled and cleared by the composition usage context menu
		std::vector<RefPtr<Graphics::AetLayer>*> compositionUsagesBuffer;

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
		void DrawTreeNodeComposition(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetComposition>& comp, bool isRoot);

		void DrawTreeNodeLayer(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetComposition>& comp, const RefPtr<Graphics::AetLayer>& layer);
		void DrawTreeNodeLayerCameraSelectableButton(const RefPtr<Graphics::AetComposition>& comp, const RefPtr<Graphics::AetLayer>& layer);
		void DrawTreeNodeLayerActivityButton(const RefPtr<Graphics::AetLayer>& layer);

		void DrawTreeNodeSurface(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetSurface>& surface, int32_t index);

		bool DrawCompositionContextMenu(const RefPtr<Graphics::Aet>& aet, const RefPtr<Graphics::AetComposition>& comp, bool isRoot);
		bool DrawLayerContextMenu(const RefPtr<Graphics::AetComposition>& comp, const RefPtr<Graphics::AetLayer>& layer);

		void DrawCompositionPreviewTooltip(const RefPtr<Graphics::AetComposition>& comp);
		void DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const;

	private:
		const char* FormatSurfaceNodeName(const RefPtr<Graphics::AetSurface>& surface, int32_t index);

	private:
		void UpdateScrollButtonInput();
		void ScrollToGuiData(GuiExtraData& guiData);

	private:
		const char* GetDebugLayerName();
	};
}

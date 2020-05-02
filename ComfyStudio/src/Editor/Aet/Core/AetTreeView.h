#pragma once
#include "Types.h"
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
		bool DrawGui(const std::shared_ptr<Graphics::Aet::AetSet>& aetSet);

		template <typename T>
		void SetSelectedItems(const std::shared_ptr<T>& value) 
		{ 
			selectedAetItem->SetItem(value);
			cameraSelectedAetItem->SetItem(value);
		}

		inline void SetSelectedItems(const std::shared_ptr<Graphics::Aet::Layer>& selectedLayer, const std::shared_ptr<Graphics::Aet::Composition>& visibleComp)
		{
			selectedAetItem->SetItem(selectedLayer);
			cameraSelectedAetItem->SetItem(visibleComp);
		}

		inline void ResetSelectedItems() 
		{ 
			selectedAetItem->Reset();
			cameraSelectedAetItem->Reset();
		}

	private:
		static constexpr const char* textureMaskIndicator = "  ( " ICON_FA_LINK " )";
		static constexpr float compPreviewTooltipHoverDelay = 0.8f;
		static constexpr int compPreviewMaxConunt = 10;

		char nodeNameFormatBuffer[512];
		
		float scrollTargetCenterRatio = 0.15f;

		std::stack<float> scrollPositionStack;

		// NOTE: To be used, filled and cleared by the composition usage context menu
		std::vector<std::shared_ptr<Graphics::Aet::Layer>*> compositionUsagesBuffer;

		struct
		{
			AetItemTypePtr* selectedAetItem;
			AetItemTypePtr* cameraSelectedAetItem;
			AetItemTypePtr hoveredAetItem, lastHoveredAetItem;
		};

		ImGuiWindow* treeViewWindow = nullptr;

	private:
		void DrawTreeViewBackground();
		
		void DrawTreeNodeAetSet(const std::shared_ptr<Graphics::Aet::AetSet>& aetSet);
		
		void DrawTreeNodeAet(const std::shared_ptr<Graphics::Aet::Scene>& scene);
		void DrawTreeNodeComposition(const std::shared_ptr<Graphics::Aet::Scene>& scene, const std::shared_ptr<Graphics::Aet::Composition>& comp, bool isRoot);

		void DrawTreeNodeLayer(const std::shared_ptr<Graphics::Aet::Scene>& scene, const std::shared_ptr<Graphics::Aet::Composition>& comp, const std::shared_ptr<Graphics::Aet::Layer>& layer);
		void DrawTreeNodeLayerCameraSelectableButton(const std::shared_ptr<Graphics::Aet::Composition>& comp, const std::shared_ptr<Graphics::Aet::Layer>& layer);
		void DrawTreeNodeLayerActivityButton(const std::shared_ptr<Graphics::Aet::Layer>& layer);

		void DrawTreeNodeVideo(const std::shared_ptr<Graphics::Aet::Scene>& scene, const std::shared_ptr<Graphics::Aet::Video>& video, i32 index);

		bool DrawCompositionContextMenu(const std::shared_ptr<Graphics::Aet::Scene>& scene, const std::shared_ptr<Graphics::Aet::Composition>& comp, bool isRoot);
		bool DrawLayerContextMenu(const std::shared_ptr<Graphics::Aet::Composition>& comp, const std::shared_ptr<Graphics::Aet::Layer>& layer);

		void DrawCompositionPreviewTooltip(const std::shared_ptr<Graphics::Aet::Composition>& comp);
		void DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const;

	private:
		const char* FormatVideoNodeName(const std::shared_ptr<Graphics::Aet::Video>& video, i32 index);

	private:
		void UpdateScrollButtonInput();
		void ScrollToGuiData(GuiExtraData& guiData);

	private:
		const char* GetDebugLayerName();
	};
}

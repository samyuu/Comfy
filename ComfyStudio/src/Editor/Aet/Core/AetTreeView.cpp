#include "AetTreeView.h"
#include "Editor/Aet/Command/Commands.h"
#include "Input/KeyCode.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	constexpr ImGuiTreeNodeFlags SelectableTreeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags HeaderTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | SelectableTreeNodeFlags;
	constexpr ImGuiTreeNodeFlags TreeNodeLeafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	AetTreeView::AetTreeView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem)
		: IMutatingEditorComponent(commandManager), selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem)
	{
		assert(selectedAetItem);
		assert(cameraSelectedAetItem);
	}

	AetTreeView::~AetTreeView()
	{
	}

	void AetTreeView::Initialize()
	{
	}

	bool AetTreeView::DrawGui(const RefPtr<AetSet>& aetSet)
	{
		treeViewWindow = Gui::GetCurrentWindow();

		if (aetSet == nullptr)
		{
			if (!scrollPositionStack.empty())
				scrollPositionStack = {};

			return false;
		}

		lastHoveredAetItem = hoveredAetItem;
		hoveredAetItem.Reset();

		// DEBUG: For developement only
		if (selectedAetItem->IsNull() && GetDebugLayerName() != nullptr)
			SetSelectedItems(aetSet->front()->FindLayer(GetDebugLayerName()));

		UpdateScrollButtonInput();

		Gui::PushStyleColor(ImGuiCol_Header, GetColor(EditorColor_TreeViewSelected));
		Gui::PushStyleColor(ImGuiCol_HeaderHovered, GetColor(EditorColor_TreeViewHovered));
		Gui::PushStyleColor(ImGuiCol_HeaderActive, GetColor(EditorColor_TreeViewActive));
		{
			DrawTreeViewBackground();
			DrawTreeNodeAetSet(aetSet);
		}
		Gui::PopStyleColor(3);

		return true;
	}

	void AetTreeView::DrawTreeViewBackground()
	{
		const ImU32 alternativeRowColow = GetColor(EditorColor_AltRow);

		const float itemSpacing = Gui::GetStyle().ItemSpacing.y;
		const float lineHeight = Gui::GetTextLineHeight() + itemSpacing;

		const ImGuiWindow* window = Gui::GetCurrentWindowRead();

		float scrollY = window->Scroll.y;
		const float scrolledOutLines = floorf(scrollY / lineHeight);
		scrollY -= lineHeight * scrolledOutLines;

		const vec2 clipRectMin = vec2(Gui::GetWindowPos().x, Gui::GetWindowPos().y);
		const vec2 clipRectMax = vec2(clipRectMin.x + Gui::GetWindowWidth(), clipRectMin.y + Gui::GetWindowHeight());

		const float yMin = clipRectMin.y - scrollY + Gui::GetCursorPosY();
		const float yMax = clipRectMax.y - scrollY + lineHeight;
		const float xMin = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Min.x - window->Pos.x;
		const float xMax = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Max.x - window->Pos.x;

		bool isOdd = fmod(scrolledOutLines, 2.0f) == 0.0f;
		for (float y = yMin; y < yMax; y += lineHeight)
		{
			if (y == yMin)
				y -= itemSpacing * 0.5f;

			if (isOdd ^= true)
				window->DrawList->AddRectFilled(vec2(xMin, y), vec2(xMax, y + lineHeight), alternativeRowColow);
		}
	}

	void AetTreeView::DrawTreeNodeAetSet(const RefPtr<AetSet>& aetSet)
	{
		const bool aetSetNodeOpen = Gui::WideTreeNodeEx(aetSet.get(), HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str());
		Gui::ItemContextMenu("AetSettAetContextMenu##AetTreeView", [this, &aetSet]()
		{
			Gui::Text("AetSet: %s", aetSet->Name.c_str());
			Gui::Separator();

			if (Gui::MenuItem("Save", nullptr, nullptr, false))
			{
				// TEMP: Don't wanna overwrite files during early development stage
				// aetSet->Save(sourceFilePath);
			}

			if (Gui::MenuItem("Save As..."))
			{
				std::wstring filePath;
				if (FileSystem::CreateSaveFileDialog(filePath, "Save AetSet file", "dev_ram/aetset", { "AetSet (*.bin)", "*.bin", "All Files (*.*)", "*", }))
					aetSet->Save(filePath);
			}
		});

		if (aetSetNodeOpen)
		{
			if (Gui::IsItemClicked())
				SetSelectedItems(aetSet);

			for (RefPtr<Aet>& aet : *aetSet)
				DrawTreeNodeAet(aet);

			Gui::TreePop();
		}
		else
		{
			ResetSelectedItems();
		}
	}

	void AetTreeView::DrawTreeNodeAet(const RefPtr<Aet>& aet)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (aet.get() == selectedAetItem->Ptrs.Aet || aet.get() == lastHoveredAetItem.Ptrs.Aet)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		const bool aetNodeOpen = Gui::WideTreeNodeEx(aet.get(), aetNodeFlags, "Aet: %s", aet->Name.c_str());

		if (Gui::IsItemClicked())
			SetSelectedItems(aet);

		if (aetNodeOpen)
		{
			if (Gui::WideTreeNodeEx(ICON_AETCOMPS "  Compositions", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				aet->RootComposition->GuiData.ThisIndex = -1;
				DrawTreeNodeComposition(aet, aet->RootComposition, true);

				for (int32_t i = static_cast<int32_t>(aet->Compositions.size()) - 1; i >= 0; i--)
				{
					const auto& comp = aet->Compositions[i];

					comp->GuiData.ThisIndex = i;
					DrawTreeNodeComposition(aet, comp, false);
				}

				Gui::TreePop();
			}

			if (Gui::WideTreeNodeEx(ICON_AETSURFACES "  Surfaces", SelectableTreeNodeFlags))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				for (int32_t i = 0; i < static_cast<int32_t>(aet->Surfaces.size()); i++)
				{
					DrawTreeNodeSurface(aet, aet->Surfaces[i], i);
				}
				Gui::TreePop();
			}

			Gui::TreePop();
		}
	}

	void AetTreeView::DrawTreeNodeComposition(const RefPtr<Aet>& aet, const RefPtr<AetComposition>& comp, bool isRoot)
	{
		Gui::PushID(comp.get());

		ImGuiTreeNodeFlags compNodeFlags = SelectableTreeNodeFlags;
		if (comp.get() == selectedAetItem->Ptrs.Composition)
			compNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (comp->size() < 1)
			compNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		comp->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

		const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

		Gui::SetNextTreeNodeOpen(comp->GuiData.TreeViewNodeOpen);
		comp->GuiData.TreeViewNodeOpen = Gui::WideTreeNodeEx("##AetCompNode", compNodeFlags);

		Gui::ItemContextMenu("AetCompContextMenu##AetTreeView", [&]()
		{
			DrawCompositionContextMenu(aet, comp, isRoot);
		});

		// TODO: Might want to check for mouse released instead (becomes more relevant once TreeNode drag and dropping is implemented)
		if (Gui::IsItemClicked())
			SetSelectedItems(comp);

		bool textHightlighted = false;
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::Layer)
			textHightlighted = (comp.get() == selectedAetItem->GetLayerRef()->GetReferencedComposition().get());

		if (textHightlighted)
			Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TreeViewTextHighlight));

		// NOTE: Node label
		{
			const vec2 nodeLabelCursorPos = treeNodeCursorPos + vec2(GImGui->FontSize + GImGui->Style.FramePadding.x, 0.0f);
			constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

			const char* compNameStart = comp->GetName().c_str();
			const char* compNameEnd = compNameStart + comp->GetName().size();

			// NOTE: Composition icon
			Gui::SetCursorScreenPos(nodeLabelCursorPos);
			Gui::TextUnformatted(comp->GuiData.TreeViewNodeOpen ? ICON_AETCOMP_OPEN : ICON_AETCOMP);

			// NOTE: Composition name
			Gui::SetCursorScreenPos(nodeLabelCursorPos + iconLabelOffset);
			Gui::TextUnformatted(compNameStart, compNameEnd);
		}

		if (textHightlighted)
			Gui::PopStyleColor();

		if (cameraSelectedAetItem->Ptrs.Composition == comp.get())
			DrawTreeNodeCameraIcon(treeNodeCursorPos);

		if (comp->GuiData.TreeViewNodeOpen)
		{
			for (RefPtr<AetLayer>& layer : *comp)
				DrawTreeNodeLayer(aet, comp, layer);

			Gui::TreePop();
		}

		Gui::PopID();
	}

	void AetTreeView::DrawTreeNodeLayer(const RefPtr<Aet>& aet, const RefPtr<AetComposition>& comp, const RefPtr<AetLayer>& layer)
	{
		Gui::PushID(layer.get());
		{
			DrawTreeNodeLayerCameraSelectableButton(comp, layer);
			DrawTreeNodeLayerActivityButton(layer);

			const bool isSelected = layer.get() == selectedAetItem->Ptrs.Layer || layer.get() == hoveredAetItem.Ptrs.Layer;
			const bool isCameraSelected = layer.get() == cameraSelectedAetItem->Ptrs.Layer;

			const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

			if (Gui::Selectable("##LayerNode", isSelected) && !isCameraSelected)
				SetSelectedItems(layer, comp);

			Gui::ItemContextMenu("LayerContextMenu##AetInspector", [this, &comp, &layer]()
			{
				DrawLayerContextMenu(comp, layer);
			});

			// NOTE: Node label
			{
				constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

				const char* layerNameStart = layer->GetName().c_str();
				const char* layerNameEnd = layerNameStart + layer->GetName().size();

				// NOTE: Composition icon
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::TextUnformatted(GetLayerTypeIcon(layer->Type));

				// NOTE: Composition name
				Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset);
				Gui::TextUnformatted(layerNameStart, layerNameEnd);

				// NOTE: Texture mask indicator
				if (layer->AnimationData != nullptr && layer->AnimationData->UseTextureMask)
				{
					Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset + vec2(Gui::CalcTextSize(layerNameStart, layerNameEnd).x, 0.0f));
					Gui::TextUnformatted(textureMaskIndicator);
				}
			}

			if (cameraSelectedAetItem->Ptrs.Layer == layer.get())
				DrawTreeNodeCameraIcon(treeNodeCursorPos);

			if (layer->Type == AetLayerType::Eff && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, compPreviewTooltipHoverDelay) && layer->GetReferencedComposition())
				DrawCompositionPreviewTooltip(layer->GetReferencedComposition());

			layer->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

			if (layer->Type == AetLayerType::Eff && (Gui::IsItemHovered() || layer.get() == selectedAetItem->Ptrs.Layer))
				hoveredAetItem.SetItem(layer->GetReferencedComposition());
		}
		Gui::PopID();
	}

	void AetTreeView::DrawTreeNodeLayerCameraSelectableButton(const RefPtr<AetComposition>& comp, const RefPtr<AetLayer>& layer)
	{
		// TODO: Does not work 100% correctly with all style settings but should be fine for now

		const vec2 cursorPos = Gui::GetCursorScreenPos();
		Gui::SetCursorScreenPos(vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, cursorPos.y - 1.0f));
		{
			bool isCameraSelected = layer.get() == cameraSelectedAetItem->Ptrs.Layer;

			const vec2 smallButtonPosition = Gui::GetCursorScreenPos();
			const vec2 smallButtonSize = vec2(cursorPos.x - smallButtonPosition.x, GImGui->FontSize + GImGui->Style.ItemSpacing.y);

			if (Gui::InvisibleButton(ICON_CAMERA, smallButtonSize))
			{
				if (isCameraSelected)
					SetSelectedItems(layer, comp);
				else
					SetSelectedItems(layer);
			}

			// TODO: It'd be nice to have some visual feedback for the hovered item inside the render window
			if (Gui::IsItemHovered())
				Gui::GetWindowDrawList()->AddText(smallButtonPosition + vec2(0.0f, 1.0f), Gui::GetColorU32(ImGuiCol_TextDisabled), ICON_CAMERA);
		}
		Gui::SetCursorScreenPos(cursorPos);
	}

	void AetTreeView::DrawTreeNodeLayerActivityButton(const RefPtr<AetLayer>& layer)
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(3.0f, 0.0f));

		ImU32 activeButtonBackgroundColor = Gui::GetColorU32(ImGuiCol_ChildBg, 0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonActive, activeButtonBackgroundColor);

		const vec2 smallButtonSize = vec2(26.0f, 0.0f);
		if (layer->Type == AetLayerType::Aif)
		{
			if (Gui::ComfySmallButton(layer->GetIsAudible() ? ICON_AUDIBLE : ICON_INAUDIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeFlagsAudible, layer, !layer->GetIsAudible());
		}
		else
		{
			if (Gui::ComfySmallButton(layer->GetIsVisible() ? ICON_VISIBLE : ICON_INVISIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), LayerChangeFlagsVisible, layer, !layer->GetIsVisible());
		}
		Gui::SameLine();

		Gui::PopStyleColor(3);
		Gui::PopStyleVar(1);
	}

	void AetTreeView::DrawTreeNodeSurface(const RefPtr<Aet>& aet, const RefPtr<AetSurface>& surface, int32_t index)
	{
		Gui::PushID(surface.get());

		bool isSelected = surface.get() == selectedAetItem->Ptrs.Surface;

		if (Gui::WideTreeNodeEx(FormatSurfaceNodeName(surface, index), TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)))
		{
			if (Gui::IsItemClicked())
				SetSelectedItems(surface);
		}

		Gui::PopID();
	}

	bool AetTreeView::DrawCompositionContextMenu(const RefPtr<Aet>& aet, const RefPtr<AetComposition>& comp, bool isRoot)
	{
		if (isRoot)
			Gui::Text(ICON_AETCOMP "  %s", comp->GetName().c_str());
		else
			Gui::Text(ICON_AETCOMP "  %s (Comp %d)", comp->GetName().c_str(), comp->GuiData.ThisIndex);
		Gui::Separator();

		constexpr bool todoImplemented = false;
		if (todoImplemented && Gui::BeginMenu(ICON_ADD "  Add new Layer..."))
		{
			// TODO: Or maybe this should only be doable inside the timeline itself (?)
			if (Gui::MenuItem(ICON_AETLAYERPIC "  Image")) {}
			if (Gui::MenuItem(ICON_AETLAYEREFF "  Comp")) {}
			if (Gui::MenuItem(ICON_AETLAYERAIF "  Sound Effect")) {}
			Gui::EndMenu();
		}

		if (Gui::BeginMenu(ICON_FA_EXTERNAL_LINK_ALT "  Used by...", !isRoot))
		{
			compositionUsagesBuffer.clear();
			AetMgr::FindAddCompositionUsages(aet, comp, compositionUsagesBuffer);

			// NOTE: Count menu item
			Gui::Text("Usage Count: %zu", compositionUsagesBuffer.size());

			// NOTE: Usage menu items
			for (const auto& compUsingLayerPointer : compositionUsagesBuffer)
			{
				const RefPtr<AetLayer>& compUsingLayer = *compUsingLayerPointer;

				sprintf_s(nodeNameFormatBuffer,
					ICON_AETCOMP "  %s,   %s  %s",
					compUsingLayer->GetParentComposition()->GetName().c_str(),
					GetLayerTypeIcon(compUsingLayer->Type),
					compUsingLayer->GetName().c_str());

				if (Gui::MenuItem(nodeNameFormatBuffer))
				{
					SetSelectedItems(compUsingLayer);
					// NOTE: Scroll to parent first to open the node
					ScrollToGuiData(compUsingLayer->GetParentComposition()->GuiData);
					// BUG: The scroll y position won't yet have been set if the comp node was previously closed
					ScrollToGuiData(compUsingLayer->GuiData);
				}
			}

			Gui::EndMenu();
		}

		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Composition", nullptr, nullptr, !isRoot && todoImplemented)) {} 

		return false;
	}

	bool AetTreeView::DrawLayerContextMenu(const RefPtr<AetComposition>& comp, const RefPtr<AetLayer>& layer)
	{
		Gui::Text("%s  %s", GetLayerTypeIcon(layer->Type), layer->GetName().c_str());

		if (layer->Type == AetLayerType::Eff && layer->GetReferencedComposition())
		{
			if (Gui::MenuItem(ICON_FA_ARROW_RIGHT "  Jump to Composition"))
			{
				ScrollToGuiData(layer->GetReferencedComposition()->GuiData);
				// NOTE: Make it clear which comp was the jump target
				SetSelectedItems(layer);
			}
		}

		Gui::Separator();

		// TODO:
		constexpr bool todoImplemented = false;
		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Layer", nullptr, nullptr, todoImplemented)) {}

		return false;
	}

	void AetTreeView::DrawCompositionPreviewTooltip(const RefPtr<AetComposition>& comp)
	{
		Gui::WideTooltip([this, &comp]()
		{
			Gui::Text(ICON_AETCOMP "  %s (Comp %d)", comp->GetName().c_str(), comp->GuiData.ThisIndex);
			Gui::Separator();

			int compIndex = 0;

			for (auto& layer : *comp)
			{
				if (compIndex++ > compPreviewMaxConunt)
				{
					Gui::Text(ICON_FA_ELLIPSIS_H);
					break;
				}

				Gui::Text("%s  %s", GetLayerTypeIcon(layer->Type), layer->GetName().c_str());
			}
		});
	}

	void AetTreeView::DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const
	{
		const vec2 textPosition = vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, treeNodeCursorPos.y);
		GImGui->CurrentWindow->DrawList->AddText(textPosition, Gui::GetColorU32(ImGuiCol_Text), ICON_CAMERA);
	}

	const char* AetTreeView::FormatSurfaceNodeName(const RefPtr<AetSurface>& surface, int32_t index)
	{
		if (surface->SpriteCount() >= 1)
		{
			if (surface->SpriteCount() > 1)
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETSURFACE "  %s - %s",
					surface->GetFrontSprite()->Name.c_str(),
					surface->GetBackSprite()->Name.c_str());
			}
			else
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETSURFACE "  %s",
					surface->GetFrontSprite()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(nodeNameFormatBuffer, ICON_AETPLACEHOLDER "  Surface %d (%dx%d)", index, surface->Size.x, surface->Size.y);
		}

		return nodeNameFormatBuffer;
	}

	void AetTreeView::UpdateScrollButtonInput()
	{
		if (!Gui::IsWindowFocused())
			return;

		if (Gui::IsKeyPressed(KeyCode_Escape))
		{
			if (!selectedAetItem->IsNull())
			{
				switch (selectedAetItem->Type())
				{
				case AetItemType::Composition:
					ScrollToGuiData(selectedAetItem->GetAetCompositionRef()->GuiData);
					break;
				case AetItemType::Layer:
					ScrollToGuiData(selectedAetItem->GetLayerRef()->GuiData);
					break;
				default:
					break;
				}
			}
		}

		// NOTE: Mouse side button to jump to last scroll position
		if (Gui::IsMouseClicked(3))
		{
			if (!scrollPositionStack.empty())
			{
				Gui::SetScrollY(scrollPositionStack.top());
				scrollPositionStack.pop();
			}
		}
	}

	void AetTreeView::ScrollToGuiData(GuiExtraData& guiData)
	{
		scrollPositionStack.push(treeViewWindow->Scroll.y);

		treeViewWindow->ScrollTarget.y = guiData.TreeViewScrollY;
		treeViewWindow->ScrollTargetCenterRatio.y = scrollTargetCenterRatio;

		guiData.TreeViewNodeOpen = true;
	}

	const char* AetTreeView::GetDebugLayerName()
	{
		return nullptr;
	}
}

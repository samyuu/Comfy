#include "AetTreeView.h"
#include "Editor/Aet/Command/Commands.h"
#include "Input/KeyCode.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Editor
{
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
		if (selectedAetItem->IsNull() && GetDebugObjectName() != nullptr)
			SetSelectedItems(aetSet->front()->FindObj(GetDebugObjectName()));

		UpdateScrollButtonInput();

		Gui::PushStyleColor(ImGuiCol_Header, GetColor(EditorColor_TreeViewSelected));
		Gui::PushStyleColor(ImGuiCol_HeaderHovered, GetColor(EditorColor_TreeViewHovered));
		Gui::PushStyleColor(ImGuiCol_HeaderActive, GetColor(EditorColor_TreeViewActive));
		{
			DrawTreeViewBackground();
			DrawTreeViewAetSet(aetSet);
		}
		Gui::PopStyleColor(3);

		return true;
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
				case AetItemType::AetLayer:
					ScrollToGuiData(selectedAetItem->GetAetLayerRef()->GuiData);
					break;
				case AetItemType::AetObj:
					ScrollToGuiData(selectedAetItem->GetAetObjRef()->GuiData);
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

	void AetTreeView::DrawTreeViewAetSet(const RefPtr<AetSet>& aetSet)
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
				DrawTreeViewAet(aet);

			Gui::TreePop();
		}
		else
		{
			ResetSelectedItems();
		}
	}

	void AetTreeView::DrawTreeViewAet(const RefPtr<Aet>& aet)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (aet.get() == selectedAetItem->Ptrs.Aet || aet.get() == lastHoveredAetItem.Ptrs.Aet)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		const bool aetNodeOpen = Gui::WideTreeNodeEx(aet.get(), aetNodeFlags, "Aet: %s", aet->Name.c_str());

		if (Gui::IsItemClicked())
			SetSelectedItems(aet);

		if (aetNodeOpen)
		{
			if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				aet->RootLayer->GuiData.ThisIndex = -1;
				DrawTreeViewLayer(aet, aet->RootLayer, true);

				for (int32_t i = static_cast<int32_t>(aet->Layers.size()) - 1; i >= 0; i--)
				{
					const auto& layer = aet->Layers[i];

					layer->GuiData.ThisIndex = i;
					DrawTreeViewLayer(aet, layer, false);
				}

				Gui::TreePop();
			}

			if (Gui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				for (int32_t i = 0; i < static_cast<int32_t>(aet->Regions.size()); i++)
				{
					DrawTreeViewRegion(aet, aet->Regions[i], i);
				}
				Gui::TreePop();
			}

			Gui::TreePop();
		}
	}

	void AetTreeView::DrawTreeViewLayer(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, bool isRoot)
	{
		Gui::PushID(aetLayer.get());

		ImGuiTreeNodeFlags layerNodeFlags = SelectableTreeNodeFlags;
		if (aetLayer.get() == selectedAetItem->Ptrs.AetLayer)
			layerNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (aetLayer->size() < 1)
			layerNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		aetLayer->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

		const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

		Gui::SetNextTreeNodeOpen(aetLayer->GuiData.TreeViewNodeOpen);
		aetLayer->GuiData.TreeViewNodeOpen = Gui::WideTreeNodeEx("##AetLayerNode", layerNodeFlags);

		bool openAddAetObjPopup = false;
		Gui::ItemContextMenu("AetLayerContextMenu##AetTreeView", [this, &aet, &aetLayer, &openAddAetObjPopup, isRoot]()
		{
			openAddAetObjPopup = DrawAetLayerContextMenu(aet, aetLayer, isRoot);
		});

		// TODO: Might want to check for mouse released instead (becomes more relevant once TreeNode drag and dropping is implemented)
		if (Gui::IsItemClicked())
			SetSelectedItems(aetLayer);

		bool textHightlighted = false;
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetItemType::AetObj)
			textHightlighted = (aetLayer.get() == selectedAetItem->GetAetObjRef()->GetReferencedLayer().get());

		if (textHightlighted)
			Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TreeViewTextHighlight));

		// NOTE: Node label
		{
			const vec2 nodeLabelCursorPos = treeNodeCursorPos + vec2(GImGui->FontSize + GImGui->Style.FramePadding.x, 0.0f);
			constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

			const char* layerNameStart = aetLayer->GetName().c_str();
			const char* layerNameEnd = layerNameStart + aetLayer->GetName().size();

			// NOTE: Layer icon
			Gui::SetCursorScreenPos(nodeLabelCursorPos);
			Gui::TextUnformatted(aetLayer->GuiData.TreeViewNodeOpen ? ICON_AETLAYER_OPEN : ICON_AETLAYER);

			// NOTE: Layer name
			Gui::SetCursorScreenPos(nodeLabelCursorPos + iconLabelOffset);
			Gui::TextUnformatted(layerNameStart, layerNameEnd);
		}

		if (textHightlighted)
			Gui::PopStyleColor();

		if (cameraSelectedAetItem->Ptrs.AetLayer == aetLayer.get())
			DrawTreeNodeCameraIcon(treeNodeCursorPos);

		if (openAddAetObjPopup)
		{
			Gui::OpenPopup(addAetObjPopupID);
			*addAetObjDialog.GetIsGuiOpenPtr() = true;
		}

		if (Gui::BeginPopupModal(addAetObjPopupID, addAetObjDialog.GetIsGuiOpenPtr(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGuiViewport* viewPort = Gui::GetMainViewport();
			ImGuiWindow* window = Gui::FindWindowByName(addAetObjPopupID);
			Gui::SetWindowPos(window, viewPort->Pos + viewPort->Size / 8, ImGuiCond_Always);
			Gui::SetWindowSize(window, viewPort->Size * 0.75f, ImGuiCond_Always);

			if (Gui::IsKeyPressed(KeyCode_Escape))
				Gui::CloseCurrentPopup();

			// NOTE: Should be replaced by drag and dropping sprites into the viewport or linking layers into eff objects (auto center origin)
			// addAetObjDialog.DrawGui(&aet, &aetLayer);
			Gui::EndPopup();
		}

		if (aetLayer->GuiData.TreeViewNodeOpen)
		{
			for (RefPtr<AetObj>& aetObj : *aetLayer)
				DrawTreeViewObj(aet, aetLayer, aetObj);

			Gui::TreePop();
		}

		Gui::PopID();
	}

	void AetTreeView::DrawTreeViewObj(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj)
	{
		Gui::PushID(aetObj.get());
		{
			DrawTreeViewObjCameraSelectableButton(aetLayer, aetObj);
			DrawTreeViewObjActivityButton(aetObj);

			const bool isSelected = aetObj.get() == selectedAetItem->Ptrs.AetObj || aetObj.get() == hoveredAetItem.Ptrs.AetObj;
			const bool isCameraSelected = aetObj.get() == cameraSelectedAetItem->Ptrs.AetObj;

			const vec2 treeNodeCursorPos = Gui::GetCursorScreenPos();

			if (Gui::Selectable("##AetObjNode", isSelected) && !isCameraSelected)
				SetSelectedItems(aetObj, aetLayer);

			Gui::ItemContextMenu("AetObjContextMenu##AetInspector", [this, &aetLayer, &aetObj]()
			{
				DrawAetObjContextMenu(aetLayer, aetObj);
			});

			// NOTE: Node label
			{
				constexpr vec2 iconLabelOffset = vec2(20.0f, 0.0f);

				const char* objNameStart = aetObj->GetName().c_str();
				const char* objNameEnd = objNameStart + aetObj->GetName().size();

				// NOTE: Layer icon
				Gui::SetCursorScreenPos(treeNodeCursorPos);
				Gui::TextUnformatted(GetObjTypeIcon(aetObj->Type));

				// NOTE: Layer name
				Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset);
				Gui::TextUnformatted(objNameStart, objNameEnd);

				// NOTE: Texture mask indicator
				if (aetObj->AnimationData != nullptr && aetObj->AnimationData->UseTextureMask)
				{
					Gui::SetCursorScreenPos(treeNodeCursorPos + iconLabelOffset + vec2(Gui::CalcTextSize(objNameStart, objNameEnd).x, 0.0f));
					Gui::TextUnformatted(textureMaskIndicator);
				}
			}

			if (cameraSelectedAetItem->Ptrs.AetObj == aetObj.get())
				DrawTreeNodeCameraIcon(treeNodeCursorPos);

			if (aetObj->Type == AetObjType::Eff && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, layerPreviewTooltipHoverDelay) && aetObj->GetReferencedLayer())
				DrawAetLayerPreviewTooltip(aetObj->GetReferencedLayer());

			aetObj->GuiData.TreeViewScrollY = Gui::GetCursorPos().y;

			if (aetObj->Type == AetObjType::Eff && (Gui::IsItemHovered() || aetObj.get() == selectedAetItem->Ptrs.AetObj))
				hoveredAetItem.SetItem(aetObj->GetReferencedLayer());
		}
		Gui::PopID();
	}

	void AetTreeView::DrawTreeViewObjCameraSelectableButton(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj)
	{
		// TODO: Does not work 100% correctly with all style settings but should be fine for now

		const vec2 cursorPos = Gui::GetCursorScreenPos();
		Gui::SetCursorScreenPos(vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, cursorPos.y - 1.0f));
		{
			bool isCameraSelected = aetObj.get() == cameraSelectedAetItem->Ptrs.AetObj;

			const vec2 smallButtonPosition = Gui::GetCursorScreenPos();
			const vec2 smallButtonSize = vec2(cursorPos.x - smallButtonPosition.x, GImGui->FontSize + GImGui->Style.ItemSpacing.y);

			if (Gui::InvisibleButton(ICON_CAMERA, smallButtonSize))
			{
				if (isCameraSelected)
					SetSelectedItems(aetObj, aetLayer);
				else
					SetSelectedItems(aetObj);
			}

			// TODO: It'd be nice to have some visual feedback for the hovered item inside the render window
			if (Gui::IsItemHovered())
				Gui::GetWindowDrawList()->AddText(smallButtonPosition + vec2(0.0f, 1.0f), Gui::GetColorU32(ImGuiCol_TextDisabled), ICON_CAMERA);
		}
		Gui::SetCursorScreenPos(cursorPos);
	}

	void AetTreeView::DrawTreeViewObjActivityButton(const RefPtr<AetObj>& aetObj)
	{
		Gui::PushStyleVar(ImGuiStyleVar_ItemSpacing, vec2(3.0f, 0.0f));

		ImU32 activeButtonBackgroundColor = Gui::GetColorU32(ImGuiCol_ChildBg, 0.0f);
		Gui::PushStyleColor(ImGuiCol_Button, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonHovered, activeButtonBackgroundColor);
		Gui::PushStyleColor(ImGuiCol_ButtonActive, activeButtonBackgroundColor);

		const vec2 smallButtonSize = vec2(26.0f, 0.0f);
		if (aetObj->Type == AetObjType::Aif)
		{
			if (Gui::ComfySmallButton(aetObj->GetIsAudible() ? ICON_AUDIBLE : ICON_INAUDIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeFlagsAudible, aetObj, !aetObj->GetIsAudible());
		}
		else
		{
			if (Gui::ComfySmallButton(aetObj->GetIsVisible() ? ICON_VISIBLE : ICON_INVISIBLE, smallButtonSize))
				ProcessUpdatingAetCommand(GetCommandManager(), AetObjChangeFlagsVisible, aetObj, !aetObj->GetIsVisible());
		}
		Gui::SameLine();

		Gui::PopStyleColor(3);
		Gui::PopStyleVar(1);
	}

	void AetTreeView::DrawTreeViewRegion(const RefPtr<Aet>& aet, const RefPtr<AetRegion>& region, int32_t index)
	{
		Gui::PushID(region.get());

		bool isSelected = region.get() == selectedAetItem->Ptrs.AetRegion;

		if (Gui::WideTreeNodeEx(FormatRegionNodeName(region, index), TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)))
		{
			if (Gui::IsItemClicked())
				SetSelectedItems(region);
		}

		Gui::PopID();
	}

	bool AetTreeView::DrawAetLayerContextMenu(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer, bool isRoot)
	{
		if (isRoot)
			Gui::Text(ICON_AETLAYER "  %s", aetLayer->GetName().c_str());
		else
			Gui::Text(ICON_AETLAYER "  %s (Layer %d)", aetLayer->GetName().c_str(), aetLayer->GuiData.ThisIndex);
		Gui::Separator();

		constexpr bool todoImplemented = false;
		if (todoImplemented && Gui::BeginMenu(ICON_ADD "  Add new AetObj..."))
		{
			// TODO: Or maybe this should only be doable inside the timeline itself (?)
			if (Gui::MenuItem(ICON_AETOBJPIC "  Image")) {}
			if (Gui::MenuItem(ICON_AETOBJEFF "  Layer")) {}
			if (Gui::MenuItem(ICON_AETOBJAIF "  Sound Effect")) {}
			Gui::EndMenu();
		}

		if (Gui::BeginMenu(ICON_FA_EXTERNAL_LINK_ALT "  Used by...", !isRoot))
		{
			layerUsagesBuffer.clear();
			Graphics::Auth2D::AetMgr::FindAddLayerUsages(aet, aetLayer, layerUsagesBuffer);

			// NOTE: Count menu item
			Gui::Text("Usage Count: %zu", layerUsagesBuffer.size());

			// NOTE: Usage menu items
			for (const auto& layerUsingObjectPointer : layerUsagesBuffer)
			{
				const RefPtr<AetObj>& layerUsingObject = *layerUsingObjectPointer;

				sprintf_s(nodeNameFormatBuffer,
					ICON_AETLAYER "  %s,   %s  %s",
					layerUsingObject->GetParentLayer()->GetName().c_str(),
					GetObjTypeIcon(layerUsingObject->Type),
					layerUsingObject->GetName().c_str());

				if (Gui::MenuItem(nodeNameFormatBuffer))
				{
					SetSelectedItems(layerUsingObject);
					// NOTE: Scroll to parent first to open the node
					ScrollToGuiData(layerUsingObject->GetParentLayer()->GuiData);
					// BUG: The scroll y position won't yet have been set if the layer node was previously closed
					ScrollToGuiData(layerUsingObject->GuiData);
				}
			}

			Gui::EndMenu();
		}

		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, !isRoot && todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Layer", nullptr, nullptr, !isRoot && todoImplemented)) {} 

		return false;
	}

	bool AetTreeView::DrawAetObjContextMenu(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj)
	{
		Gui::Text("%s  %s", GetObjTypeIcon(aetObj->Type), aetObj->GetName().c_str());

		if (aetObj->Type == AetObjType::Eff && aetObj->GetReferencedLayer())
		{
			if (Gui::MenuItem(ICON_FA_ARROW_RIGHT "  Jump to Layer"))
			{
				ScrollToGuiData(aetObj->GetReferencedLayer()->GuiData);
				// NOTE: Make it clear which layer was the jump target
				SetSelectedItems(aetObj);
			}
		}

		Gui::Separator();

		// TODO:
		constexpr bool todoImplemented = false;
		if (Gui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, todoImplemented)) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Object", nullptr, nullptr, todoImplemented)) {}

		return false;
	}

	void AetTreeView::DrawAetLayerPreviewTooltip(const RefPtr<AetLayer>& aetLayer)
	{
		Gui::WideTooltip([this, &aetLayer]()
		{
			Gui::Text(ICON_AETLAYER "  %s (Layer %d)", aetLayer->GetName().c_str(), aetLayer->GuiData.ThisIndex);
			Gui::Separator();

			int layerIndex = 0;

			for (auto& obj : *aetLayer)
			{
				if (layerIndex++ > layerPreviewMaxConunt)
				{
					Gui::Text(ICON_FA_ELLIPSIS_H);
					break;
				}

				Gui::Text("%s  %s", GetObjTypeIcon(obj->Type), obj->GetName().c_str());
			}
		});
	}

	void AetTreeView::DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const
	{
		const vec2 textPosition = vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, treeNodeCursorPos.y);
		GImGui->CurrentWindow->DrawList->AddText(textPosition, Gui::GetColorU32(ImGuiCol_Text), ICON_CAMERA);
	}

	const char* AetTreeView::FormatRegionNodeName(const RefPtr<AetRegion>& region, int32_t index)
	{
		if (region->SpriteCount() >= 1)
		{
			if (region->SpriteCount() > 1)
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETREGION "  %s - %s",
					region->GetFrontSprite()->Name.c_str(),
					region->GetBackSprite()->Name.c_str());
			}
			else
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETREGION "  %s",
					region->GetFrontSprite()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(nodeNameFormatBuffer, ICON_AETREGIONNOSPR "  Region %d (%dx%d)", index, region->Size.x, region->Size.y);
		}

		return nodeNameFormatBuffer;
	}

	void AetTreeView::ScrollToGuiData(GuiExtraData& guiData)
	{
		scrollPositionStack.push(treeViewWindow->Scroll.y);

		treeViewWindow->ScrollTarget.y = guiData.TreeViewScrollY;
		treeViewWindow->ScrollTargetCenterRatio.y = scrollTargetCenterRatio;

		guiData.TreeViewNodeOpen = true;
	}

	const char* AetTreeView::GetDebugObjectName()
	{
		return nullptr;
	}
}
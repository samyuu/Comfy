#include "AetTreeView.h"
#include "Editor/Aet/AetIcons.h"
#include "Input/KeyCode.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"
#include "Core/Logger.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags SelectableTreeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags HeaderTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | SelectableTreeNodeFlags;
	constexpr ImGuiTreeNodeFlags TreeNodeLeafFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

	AetTreeView::AetTreeView(AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem)
		: selectedAetItem(selectedAetItem), cameraSelectedAetItem(cameraSelectedAetItem)
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

		UpdateScrollInput();

		// NOTE: For developement only
		if (selectedAetItem->IsNull() && GetDebugObjectName() != nullptr)
			SetSelectedItems(aetSet->front()->FindObj(GetDebugObjectName()));

		DrawTreeViewBackground();

		bool aetSetNodeOpen = Gui::WideTreeNodeEx(aetSet.get(), HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str());
		Gui::ItemContextMenu("AetSettAetContextMenu##AetTreeView", [this, &aetSet]()
		{
			Gui::Text("AetSet: %s", aetSet->Name.c_str());
			Gui::Separator();

			if (Gui::MenuItem("Save", nullptr, nullptr, false))
			{
				// NOTE: Don't wanna overwrite files during early development stage
			}

			if (Gui::MenuItem("Save As..."))
			{
				WideString filePath;
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

		return true;
	}

	void AetTreeView::UpdateScrollInput()
	{
		if (!Gui::IsWindowFocused())
			return;

		if (Gui::IsKeyPressed(KeyCode_Escape))
		{
			if (!selectedAetItem->IsNull())
			{
				switch (selectedAetItem->Type())
				{
				case AetSelectionType::AetLayer:
					ScrollToGuiData(selectedAetItem->GetAetLayerRef()->GuiTempData);
					break;
				case AetSelectionType::AetObj:
					ScrollToGuiData(selectedAetItem->GetAetObjRef()->GuiTempData);
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

		float itemSpacing = Gui::GetStyle().ItemSpacing.y;
		float lineHeight = Gui::GetTextLineHeight() + itemSpacing;

		ImGuiWindow* window = Gui::GetCurrentWindowRead();

		float scrollY = window->Scroll.y;
		float scrolledOutLines = floorf(scrollY / lineHeight);
		scrollY -= lineHeight * scrolledOutLines;

		ImVec2 clipRectMin(Gui::GetWindowPos().x, Gui::GetWindowPos().y);
		ImVec2 clipRectMax(clipRectMin.x + Gui::GetWindowWidth(), clipRectMin.y + Gui::GetWindowHeight());

		float yMin = clipRectMin.y - scrollY + Gui::GetCursorPosY();
		float yMax = clipRectMax.y - scrollY + lineHeight;
		float xMin = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Min.x - window->Pos.x;
		float xMax = clipRectMin.x + window->Scroll.x + window->ContentsRegionRect.Max.x - window->Pos.x;

		bool isOdd = fmod(scrolledOutLines, 2.0f) == 0.0f;
		for (float y = yMin; y < yMax; y += lineHeight)
		{
			if (y == yMin)
				y -= itemSpacing * 0.5f;

			if (isOdd ^= true)
				window->DrawList->AddRectFilled({ xMin, y }, { xMax, y + lineHeight }, alternativeRowColow);
		}
	}

	void AetTreeView::DrawTreeViewAet(const RefPtr<Aet>& aet)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (aet.get() == selectedAetItem->Ptrs.Aet || aet.get() == lastHoveredAetItem.Ptrs.Aet)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool aetNodeOpen = Gui::WideTreeNodeEx(FormatAetNodeName(aet), aetNodeFlags);

		if (Gui::IsItemClicked())
			SetSelectedItems(aet);

		if (aetNodeOpen)
		{
			if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				for (int32_t i = static_cast<int32_t>(aet->AetLayers.size()) - 1; i >= 0; i--)
				{
					DrawTreeViewLayer(aet, aet->AetLayers[i]);
				}

				Gui::TreePop();
			}

			if (Gui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItems();

				for (int32_t i = 0; i < static_cast<int32_t>(aet->AetRegions.size()); i++)
				{
					DrawTreeViewRegion(aet, aet->AetRegions[i], i);
				}
				Gui::TreePop();
			}

			Gui::TreePop();
		}
	}

	void AetTreeView::DrawTreeViewLayer(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer)
	{
		Gui::PushID(aetLayer.get());

		ImGuiTreeNodeFlags layerNodeFlags = SelectableTreeNodeFlags;
		if (aetLayer.get() == selectedAetItem->Ptrs.AetLayer)
			layerNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (aetLayer->size() < 1)
			layerNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		if (aetLayer->GuiTempData.AppendOpenNode)
		{
			Gui::SetNextTreeNodeOpen(true);
			aetLayer->GuiTempData.AppendOpenNode = false;
		}

		ImVec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
		bool aetLayerNodeOpen = Gui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags);

		aetLayer->GuiTempData.TreeViewScrollY = Gui::GetCursorPos().y;

		bool openAddAetObjPopup = false;
		Gui::ItemContextMenu("AetLayerContextMenu##AetTreeView", [this, &aet, &aetLayer, &openAddAetObjPopup]()
		{
			openAddAetObjPopup = DrawAetLayerContextMenu(aet, aetLayer);
		});

		// TODO: Might want to check for mouse released instead (becomes more relevant once TreeNode drag and dropping is implemented)
		if (Gui::IsItemClicked())
			SetSelectedItems(aetLayer);

		bool textHightlighted = false;
		if (!selectedAetItem->IsNull() && selectedAetItem->Type() == AetSelectionType::AetObj)
			textHightlighted = aetLayer.get() == selectedAetItem->GetAetObjRef()->GetReferencedLayer().get();

		if (textHightlighted)
			Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

		Gui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(GImGui->FontSize + GImGui->Style.FramePadding.x, 0));
		Gui::TextUnformatted(FormatLayerNodeName(aetLayer, aetLayerNodeOpen));

		if (textHightlighted)
			Gui::PopStyleColor();

		// TODO: Have a separate "selected" and "cameraSelected" (double click AetObj to camera select)
		if (cameraSelectedAetItem->Ptrs.AetLayer == aetLayer.get())
			DrawTreeNodeCameraIcon(treeNodeCursorPos);

		if (openAddAetObjPopup)
		{
			Gui::OpenPopup(AddAetObjPopupID);
			*addAetObjDialog.GetIsGuiOpenPtr() = true;
		}

		if (Gui::BeginPopupModal(AddAetObjPopupID, addAetObjDialog.GetIsGuiOpenPtr(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGuiViewport* viewPort = Gui::GetMainViewport();
			ImGuiWindow* window = Gui::FindWindowByName(AddAetObjPopupID);
			Gui::SetWindowPos(window, viewPort->Pos + viewPort->Size / 8, ImGuiCond_Always);
			Gui::SetWindowSize(window, viewPort->Size * .75f, ImGuiCond_Always);

			if (Gui::IsKeyPressed(KeyCode_Escape))
				Gui::CloseCurrentPopup();

			// NOTE: Should be replaced by drag and dropping sprites into the viewport or linking layers into eff objects
			// addAetObjDialog.DrawGui(&aet, &aetLayer);
			Gui::EndPopup();
		}

		if (aetLayerNodeOpen)
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

			if (Gui::Selectable(FormatObjNodeName(aetObj), isSelected) && !isCameraSelected)
				SetSelectedItems(aetObj, aetLayer);

			if (cameraSelectedAetItem->Ptrs.AetObj == aetObj.get())
				DrawTreeNodeCameraIcon(treeNodeCursorPos);

			if (aetObj->Type == AetObjType::Eff && Gui::IsItemHoveredDelayed(ImGuiHoveredFlags_None, LayerPreviewTooltipHoverDelay) && aetObj->GetReferencedLayer())
				DrawAetLayerPreviewTooltip(aetObj->GetReferencedLayer());

			aetObj->GuiTempData.TreeViewScrollY = Gui::GetCursorPos().y;

			Gui::ItemContextMenu("AetObjContextMenu##AetInspector", [this, &aetLayer, &aetObj]()
			{
				DrawAetObjContextMenu(aetLayer, aetObj);
			});

			if (aetObj->Type == AetObjType::Eff && (Gui::IsItemHovered() || aetObj.get() == selectedAetItem->Ptrs.AetObj))
				hoveredAetItem.SetItem(aetObj->GetReferencedLayer());
		}
		Gui::PopID();
	}

	void AetTreeView::DrawTreeViewObjCameraSelectableButton(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj)
	{
		// TODO: Does not work 100% correctly with all style settings but should be fine for now

		const vec2 cursorPos = Gui::GetCursorScreenPos();
		Gui::SetCursorScreenPos(vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, cursorPos.y - 1));
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
				Gui::GetWindowDrawList()->AddText(smallButtonPosition + vec2(0.0f, 1), Gui::GetColorU32(ImGuiCol_TextDisabled), ICON_CAMERA);
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
			if (Gui::ComfySmallButton(aetObj->Flags.Audible ? ICON_AUDIBLE : ICON_INAUDIBLE, smallButtonSize))
				aetObj->Flags.Audible ^= true;
		}
		else
		{
			if (Gui::ComfySmallButton(aetObj->Flags.Visible ? ICON_VISIBLE : ICON_INVISIBLE, smallButtonSize))
				aetObj->Flags.Visible ^= true;
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

	bool AetTreeView::DrawAetLayerContextMenu(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer)
	{
		Gui::Text(ICON_AETLAYER "  %s", aetLayer->GetCommaSeparatedNames());
		Gui::Separator();

		if (Gui::BeginMenu(ICON_ADD "  Add new AetObj..."))
		{
			// TODO: loop through layer objects to search for previous null_%d objects and increment index
			if (Gui::MenuItem(ICON_AETOBJPIC "  Image")) {} // { aetLayer->AddNewObject(AetObjType::Pic, "null_00"); }
			if (Gui::MenuItem(ICON_AETOBJEFF "  Layer")) {} // { aetLayer->AddNewObject(AetObjType::Eff, "null_eff"); }
			if (Gui::MenuItem(ICON_AETOBJAIF "  Sound Effect")) {} // { aetLayer->AddNewObject(AetObjType::Aif, "null.aif"); }
			Gui::EndMenu();
		}

		if (Gui::MenuItem(ICON_MOVEUP "  Move Up")) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down")) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Layer")) {} // TODO: layerToDelete = { &aet, &aetLayer};

		return false;
	}

	bool AetTreeView::DrawAetObjContextMenu(const RefPtr<AetLayer>& aetLayer, const RefPtr<AetObj>& aetObj)
	{
		Gui::Text("%s  %s", GetObjTypeIcon(aetObj->Type), aetObj->GetName().c_str());

		if (aetObj->Type == AetObjType::Eff && aetObj->GetReferencedLayer())
		{
			if (Gui::MenuItem(ICON_FA_ARROW_RIGHT "  Jump to Layer"))
			{
				ScrollToGuiData(aetObj->GetReferencedLayer()->GuiTempData);
				// NOTE: Make it clear which layer was the jump target
				SetSelectedItems(aetObj);
			}
		}

		Gui::Separator();

		// TODO:
		if (Gui::MenuItem(ICON_MOVEUP "  Move Up")) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down")) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Object")) {} // TODO: { objToDelete = { &aetLayer, &aetObj }; }

		return false;
	}

	void AetTreeView::DrawAetLayerPreviewTooltip(const RefPtr<AetLayer>& aetLayer) const
	{
		Gui::WideTooltip([&aetLayer]()
		{
			Gui::Text("Layer %d (%s)", aetLayer->GetThisIndex(), aetLayer->GetCommaSeparatedNames());
			Gui::Separator();

			for (auto& obj : *aetLayer)
				Gui::Text("%s  %s", GetObjTypeIcon(obj->Type), obj->GetName().c_str());
		});
	}

	void AetTreeView::DrawTreeNodeCameraIcon(const vec2& treeNodeCursorPos) const
	{
		GImGui->CurrentWindow->DrawList->AddText(
			vec2(GImGui->CurrentWindow->Pos.x + GImGui->Style.FramePadding.x - GImGui->CurrentWindow->Scroll.x, treeNodeCursorPos.y),
			Gui::GetColorU32(ImGuiCol_Text),
			ICON_CAMERA);
	}

	const char* AetTreeView::FormatAetNodeName(const RefPtr<Aet>& aet)
	{
		const int nameLength = sizeof(nodeNameFormatBuffer) - 12;

		sprintf_s(nodeNameFormatBuffer,
			"Aet: %s",
			aet->Name.c_str());

		return nodeNameFormatBuffer;
	}

	const char* AetTreeView::FormatLayerNodeName(const RefPtr<AetLayer>& aetLayer, bool nodeOpen)
	{
		const int nameLength = sizeof(nodeNameFormatBuffer) - 32;

		sprintf_s(nodeNameFormatBuffer,
			"%s  Layer %d (%.*s)",
			nodeOpen ? ICON_AETLAYER_OPEN : ICON_AETLAYER,
			aetLayer->GetThisIndex(),
			nameLength,
			aetLayer->GetCommaSeparatedNames());

		return nodeNameFormatBuffer;
	}

	const char* AetTreeView::FormatObjNodeName(const RefPtr<AetObj>& aetObj)
	{
		const int nameLength = sizeof(nodeNameFormatBuffer) - 22;

		const bool useTextureMask = aetObj->AnimationData != nullptr && aetObj->AnimationData->UseTextureMask;
		const char* textureMaskIndicator = useTextureMask ? "  ( " ICON_FA_LINK " )  " : "";

		sprintf_s(nodeNameFormatBuffer,
			"%s  %.*s%s",
			GetObjTypeIcon(aetObj->Type),
			nameLength,
			aetObj->GetName().c_str(),
			textureMaskIndicator);

		return nodeNameFormatBuffer;
	}

	const char* AetTreeView::FormatRegionNodeName(const RefPtr<AetRegion>& region, int32_t index)
	{
		if (region->SpriteCount() >= 1)
		{
			if (region->SpriteCount() > 1)
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETREGION "  Region %d (%s - %s)", index,
					region->GetFrontSprite()->Name.c_str(),
					region->GetBackSprite()->Name.c_str());
			}
			else
			{
				sprintf_s(nodeNameFormatBuffer, ICON_AETREGION "  Region %d (%s)", index,
					region->GetFrontSprite()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(nodeNameFormatBuffer, ICON_AETREGIONNOSPR "  Region %d (%dx%d)", index, region->Width, region->Height);
		}

		return nodeNameFormatBuffer;
	}

	void AetTreeView::ScrollToGuiData(GuiTempData& guiData)
	{
		scrollPositionStack.push(treeViewWindow->Scroll.y);

		treeViewWindow->ScrollTarget.y = guiData.TreeViewScrollY;
		treeViewWindow->ScrollTargetCenterRatio.y = scrollTargetCenterRatio;

		guiData.AppendOpenNode = true;
	}

	const char* AetTreeView::GetDebugObjectName()
	{
		return nullptr;
	}
}
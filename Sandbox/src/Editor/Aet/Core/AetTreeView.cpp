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

	AetTreeView::AetTreeView()
	{
	}

	AetTreeView::~AetTreeView()
	{
	}

	void AetTreeView::Initialize()
	{
	}

	bool AetTreeView::DrawGui(const RefPtr<AetSet>& aetSet)
	{
		if (aetSet == nullptr)
			return false;

		lastHovered = hovered;
		hovered.Reset();

		if (selected.IsNull() && GetDebugObjectName() != nullptr)
		{
			activeAet = aetSet->front().get();
			selected.SetItem(activeAet->FindObj(GetDebugObjectName()));
		}

		DrawTreeViewBackground();

		bool aetSetNodeOpen = Gui::WideTreeNodeEx(aetSet.get(), HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str());
		Gui::ItemContextMenu("AetSettAetContextMenu##AetTreeView", [this, &aetSet]()
		{
			Gui::Text("AetSet: %s", aetSet->Name.c_str());
			Gui::Separator();

			if (Gui::MenuItem("Save", nullptr, nullptr, false))
			{
				// don't wanna overwrite files during early development stage
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
				SetSelectedItem(aetSet);

			for (RefPtr<Aet>& aet : *aetSet)
				DrawTreeViewAet(aet);

			Gui::TreePop();
		}
		else
		{
			ResetSelectedItem();
		}

		return true;
	}

	void AetTreeView::DrawTreeViewBackground()
	{
		const ImU32 color = 0xFF363636;

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
				window->DrawList->AddRectFilled({ xMin, y }, { xMax, y + lineHeight }, color);
		}
	}

	void AetTreeView::DrawTreeViewAet(const RefPtr<Aet>& aet)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (aet.get() == selected.Ptrs.Aet || aet.get() == lastHovered.Ptrs.Aet)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool aetNodeOpen = Gui::WideTreeNodeEx(aet.get(), aetNodeFlags, "Aet: %s", aet->Name.c_str());

		if (Gui::IsItemClicked())
			SetSelectedItem(aet.get(), aet);

		if (aetNodeOpen)
		{
			//Gui::PushStyleVar(ImGuiStyleVar_IndentSpacing, Gui::GetFontSize() * 1.5f);

			if (Gui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItem();

				for (int32_t i = aet->AetLayers.size() - 1; i >= 0; i--)
				{
					DrawTreeViewLayer(aet, aet->AetLayers[i]);
				}

				Gui::TreePop();
			}

			if (Gui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				if (Gui::IsItemClicked())
					ResetSelectedItem();

				for (int32_t i = 0; i < aet->AetRegions.size(); i++)
				{
					DrawTreeViewRegion(aet, aet->AetRegions[i], i);
				}
				Gui::TreePop();
			}

			//Gui::PopStyleVar();
			Gui::TreePop();
		}
	}

	void AetTreeView::DrawTreeViewLayer(const RefPtr<Aet>& aet, const RefPtr<AetLayer>& aetLayer)
	{
		Gui::PushID(&aetLayer);

		ImGuiTreeNodeFlags layerNodeFlags = SelectableTreeNodeFlags;
		if (aetLayer.get() == selected.Ptrs.AetLayer)
			layerNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (aetLayer->size() < 1)
			layerNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		ImVec2 treeNodeCursorPos = Gui::GetCursorScreenPos();
		bool aetLayerNodeOpen = Gui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags);

		bool openAddAetObjPopup = false;
		Gui::ItemContextMenu("AetLayerContextMenu##AetTreeView", [this, &aet, &aetLayer, &openAddAetObjPopup]()
		{
			openAddAetObjPopup = DrawAetLayerContextMenu(aet, aetLayer);
		});

		// TODO: should check for mouse released
		if (Gui::IsItemClicked())
			SetSelectedItem(aet.get(), aetLayer);

		bool lastHoveredLayer = aetLayer.get() == lastHovered.Ptrs.AetLayer;
		if (lastHoveredLayer)
			Gui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

		Gui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(GImGui->FontSize + GImGui->Style.FramePadding.x, 0));
		Gui::Text(
			"%s  Layer %d (%s)",
			aetLayerNodeOpen ? ICON_AETLAYER_OPEN : ICON_AETLAYER,
			aetLayer->GetThisIndex(),
			aetLayer->GetCommaSeparatedNames());

		if (lastHoveredLayer)
			Gui::PopStyleColor();

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
			Gui::SetWindowSize(window, viewPort->Size * .75f, ImGuiCond_Always);

			if (Gui::IsKeyPressed(KeyCode_Escape))
				Gui::CloseCurrentPopup();

			//addAetObjDialog.DrawGui(&aet, &aetLayer);
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
			bool isSelected = aetObj.get() == selected.Ptrs.AetObj || aetObj.get() == hovered.Ptrs.AetObj;

			bool drawActiveButton = true;
			if (drawActiveButton)
			{
				const ImVec2 smallButtonSize = ImVec2(26.0f, 0.0f);
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
			}

			sprintf_s(objNameBuffer, "%s  %s", GetObjTypeIcon(aetObj->Type), aetObj->GetName().c_str());

			if (drawActiveButton)
			{
				if (Gui::Selectable(objNameBuffer, isSelected))
					SetSelectedItem(aet.get(), aetObj);
			}
			else
			{
				Gui::WideTreeNodeEx(objNameBuffer, TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));

				if (Gui::IsItemClicked())
					SetSelectedItem(aet.get(), aetObj);
			}

			Gui::ItemContextMenu("AetObjContextMenu##AetInspector", [this, &aetLayer, &aetObj]()
			{
				DrawAetObjContextMenu(aetLayer, aetObj);
			});

			if (aetObj->Type == AetObjType::Eff && (Gui::IsItemHovered() || aetObj.get() == selected.Ptrs.AetObj))
				hovered.SetItem(aetObj->GetReferencedLayer());
		}
		Gui::PopID();
	}

	void AetTreeView::DrawTreeViewRegion(const RefPtr<Aet>& aet, const RefPtr<AetRegion>& region, int32_t index)
	{
		Gui::PushID(&region);

		if (region->SpriteSize() >= 1)
		{
			if (region->SpriteSize() > 1)
			{
				sprintf_s(regionNameBuffer, ICON_AETREGION "  Region %d (%s - %s)", index,
					region->GetFrontSprite()->Name.c_str(),
					region->GetBackSprite()->Name.c_str());
			}
			else
			{
				sprintf_s(regionNameBuffer, ICON_AETREGION "  Region %d (%s)", index,
					region->GetFrontSprite()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(regionNameBuffer, ICON_AETREGIONNOSPR "  Region %d (%dx%d)", index, region->Width, region->Height);
		}

		bool isSelected = region.get() == selected.Ptrs.AetRegion;

		if (Gui::WideTreeNodeEx(regionNameBuffer, TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)))
		{
			if (Gui::IsItemClicked())
				SetSelectedItem(aet.get(), region);
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
			if (Gui::MenuItem(ICON_AETOBJPIC "  Image")) { aetLayer->AddNewObject(AetObjType::Pic, "null_00"); }
			if (Gui::MenuItem(ICON_AETOBJEFF "  Layer")) { aetLayer->AddNewObject(AetObjType::Eff, "null_eff"); }
			if (Gui::MenuItem(ICON_AETOBJAIF "  Sound Effect")) { aetLayer->AddNewObject(AetObjType::Aif, "null.aif"); }
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
		Gui::Separator();

		// TODO:
		if (Gui::MenuItem(ICON_MOVEUP "  Move Up")) {}
		if (Gui::MenuItem(ICON_MOVEDOWN "  Move Down")) {}
		if (Gui::MenuItem(ICON_DELETE "  Delete Object")) {} // TODO: { objToDelete = { &aetLayer, &aetObj }; }

		return false;
	}

	const char* AetTreeView::GetDebugObjectName()
	{
		return nullptr;
	}
}
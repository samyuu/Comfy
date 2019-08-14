#include "AetTreeView.h"
#include "AetIcons.h"
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

	bool AetTreeView::DrawGui(AetSet* aetSet)
	{
		if (aetSet == nullptr)
			return false;

		lastHovered = hovered;
		hovered.Reset();

		if (selected.VoidPointer == nullptr && GetDebugObjectName() != nullptr)
		{
			activeAet = aetSet->front().get();
			selected.SetItem(activeAet->GetObj(GetDebugObjectName()));
		}

		DrawTreeViewBackground();

		bool aetSetNodeOpen = ImGui::WideTreeNodeEx((void*)aetSet, HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str());
		ImGui::ItemContextMenu("AetSettAetContextMenu##AetTreeView", [this, aetSet]() 
		{
			ImGui::Text("AetSet: %s", aetSet->Name.c_str());
			ImGui::Separator();

			if (ImGui::MenuItem("Save", nullptr, nullptr, false))
			{
				// don't wanna overwrite files during early development stage
			}

			if (ImGui::MenuItem("Save As..."))
			{
				std::wstring filePath;
				if (FileSystem::CreateSaveFileDialog(filePath, "Save AetSet file", "dev_ram/aetset", { "AetSet (*.bin)", "*.bin", "All Files (*.*)", "*", }))
					aetSet->Save(filePath);
			}
		});

		if (aetSetNodeOpen)
		{
			if (ImGui::IsItemClicked())
				SetSelectedItem(aetSet);

			for (RefPtr<Aet>& aet : *aetSet)
				DrawTreeViewAet(*aet);

			ImGui::TreePop();
		}
		else
		{
			ResetSelectedItem();
		}

		if (objToDelete.Layer != nullptr && objToDelete.Obj != nullptr)
		{
			if (selected.AetObj == objToDelete.Obj)
				selected.AetObj = nullptr;

			objToDelete.Layer->DeleteObject(objToDelete.Obj);
			objToDelete = { nullptr, nullptr };
		}

		return true;
	}

	void AetTreeView::DrawTreeViewBackground()
	{
		const ImU32 color = 0xFF363636;

		float itemSpacing = ImGui::GetStyle().ItemSpacing.y;
		float lineHeight = ImGui::GetTextLineHeight() + itemSpacing;

		ImGuiWindow* window = ImGui::GetCurrentWindowRead();

		float scrollY = window->Scroll.y;
		float scrolledOutLines = floorf(scrollY / lineHeight);
		scrollY -= lineHeight * scrolledOutLines;

		ImVec2 clipRectMin(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y);
		ImVec2 clipRectMax(clipRectMin.x + ImGui::GetWindowWidth(), clipRectMin.y + ImGui::GetWindowHeight());

		float yMin = clipRectMin.y - scrollY + ImGui::GetCursorPosY();
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

	void AetTreeView::DrawTreeViewAet(Aet& aet)
	{
		ImGuiTreeNodeFlags aetNodeFlags = HeaderTreeNodeFlags;
		if (&aet == selected.Aet || &aet == lastHovered.Aet)
			aetNodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool aetNodeOpen = ImGui::WideTreeNodeEx((void*)&aet, aetNodeFlags, "Aet: %s", aet.Name.c_str());

		if (ImGui::IsItemClicked())
			SetSelectedItem(&aet, &aet);

		if (aetNodeOpen)
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			if (ImGui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::IsItemClicked())
					ResetSelectedItem();

				for (RefPtr<AetLayer>& aetLayer : aet.AetLayers)
					DrawTreeViewLayer(aet, *aetLayer);

				ImGui::TreePop();
			}

			if (ImGui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				if (ImGui::IsItemClicked())
					ResetSelectedItem();

				for (int32_t i = 0; i < aet.AetRegions.size(); i++)
				{
					DrawTreeViewRegion(aet, *aet.AetRegions[i], i);
				}
				ImGui::TreePop();
			}

			//ImGui::PopStyleVar();
			ImGui::TreePop();
		}
	}

	void AetTreeView::DrawTreeViewLayer(Aet& aet, AetLayer& aetLayer)
	{
		ImGui::PushID(&aetLayer);

		ImGuiTreeNodeFlags layerNodeFlags = SelectableTreeNodeFlags;
		if (&aetLayer == selected.AetLayer)
			layerNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (aetLayer.size() < 1)
			layerNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		ImVec2 treeNodeCursorPos = ImGui::GetCursorScreenPos();
		bool aetLayerNodeOpen = ImGui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags);

		if (ImGui::IsItemClicked())
			SetSelectedItem(&aet, &aetLayer);

		bool openAddAetObjPopup = false;
		ImGui::ItemContextMenu("AetLayerContextMenu##AetTreeView", [this, &aet, &aetLayer, &openAddAetObjPopup]()
		{
			openAddAetObjPopup = DrawAetLayerContextMenu(aet, aetLayer);
		});

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

		ImGui::SetCursorScreenPos(treeNodeCursorPos + ImVec2(GImGui->FontSize + (GImGui->Style.FramePadding.x * 1), 0));
		ImGui::Text(
			"%s  Layer %d (%s)",
			aetLayerNodeOpen ? ICON_AETLAYER_OPEN : ICON_AETLAYER,
			aetLayer.GetThisIndex(),
			aetLayer.GetCommaSeparatedNames());

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PopStyleColor();

		if (openAddAetObjPopup)
		{
			ImGui::OpenPopup(addAetObjPopupID);
			*addAetObjDialog.GetIsGuiOpenPtr() = true;
		}

		if (ImGui::BeginPopupModal(addAetObjPopupID, addAetObjDialog.GetIsGuiOpenPtr(), ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGuiViewport* viewPort = ImGui::GetMainViewport();
			ImGuiWindow* window = ImGui::FindWindowByName(addAetObjPopupID);
			ImGui::SetWindowPos(window, viewPort->Pos + viewPort->Size / 8, ImGuiCond_Always);
			ImGui::SetWindowSize(window, viewPort->Size * .75f, ImGuiCond_Always);

			if (ImGui::IsKeyPressed(KeyCode_Escape))
				ImGui::CloseCurrentPopup();

			addAetObjDialog.DrawGui(&aet, &aetLayer);
			ImGui::EndPopup();
		}

		if (aetLayerNodeOpen)
		{
			for (RefPtr<AetObj>& aetObj : aetLayer)
				DrawTreeViewObj(aet, aetLayer, *aetObj);

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewObj(Aet& aet, AetLayer& aetLayer, AetObj& aetObj)
	{
		ImGui::PushID((void*)&aetObj);
		{
			bool isSelected = &aetObj == selected.AetObj || &aetObj == hovered.AetObj;

			bool drawActiveButton = true;
			if (drawActiveButton)
			{
				const ImVec2 smallButtonSize = ImVec2(26.0f, 0.0f);
				if (aetObj.Type == AetObjType::Aif)
				{
					if (ImGui::SmallButton(aetObj.Flags.Audible ? ICON_AUDIBLE : ICON_INAUDIBLE, smallButtonSize))
						aetObj.Flags.Audible ^= true;
				}
				else
				{
					if (ImGui::SmallButton(aetObj.Flags.Visible ? ICON_VISIBLE : ICON_INVISIBLE, smallButtonSize))
						aetObj.Flags.Visible ^= true;
				}
				ImGui::SameLine();
			}

			sprintf_s(objNameBuffer, "%s  %s", GetObjTypeIcon(aetObj.Type), aetObj.GetName().c_str());

			if (drawActiveButton)
			{
				if (ImGui::Selectable(objNameBuffer, isSelected))
					SetSelectedItem(&aet, &aetObj);
			}
			else
			{
				ImGui::WideTreeNodeEx(objNameBuffer, TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None));

				if (ImGui::IsItemClicked())
					SetSelectedItem(&aet, &aetObj);
			}

			ImGui::ItemContextMenu("AetObjContextMenu##AetInspector", [this, &aetLayer, &aetObj]()
			{
				DrawAetObjContextMenu(aetLayer, aetObj);
			});

			if (aetObj.Type == AetObjType::Eff && (ImGui::IsItemHovered() || &aetObj == selected.AetObj))
				hovered.SetItem(aetObj.GetReferencedLayer());
		}
		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewRegion(Aet& aet, AetRegion& region, int32_t index)
	{
		ImGui::PushID(&region);

		if (region.SpriteSize() >= 1)
		{
			if (region.SpriteSize() > 1)
			{
				sprintf_s(regionNameBuffer, ICON_AETREGION "  Region %d (%s - %s)", index,
					region.GetFrontSprite()->Name.c_str(),
					region.GetBackSprite()->Name.c_str());
			}
			else
			{
				sprintf_s(regionNameBuffer, ICON_AETREGION "  Region %d (%s)", index,
					region.GetFrontSprite()->Name.c_str());
			}
		}
		else
		{
			sprintf_s(regionNameBuffer, ICON_AETREGIONNOSPR "  Region %d (%dx%d)", index, region.Width, region.Height);
		}

		bool isSelected = &region == selected.AetRegion;

		if (ImGui::WideTreeNodeEx(regionNameBuffer, TreeNodeLeafFlags | (isSelected ? ImGuiTreeNodeFlags_Selected : ImGuiTreeNodeFlags_None)))
		{
			if (ImGui::IsItemClicked())
				SetSelectedItem(&aet, &region);
		}

		ImGui::PopID();
	}

	bool AetTreeView::DrawAetLayerContextMenu(Aet& aet, AetLayer& aetLayer)
	{
		ImGui::Text(ICON_AETLAYER "  %s", aetLayer.GetCommaSeparatedNames());
		ImGui::Separator();

		if (ImGui::BeginMenu(ICON_ADD "  Add new AetObj..."))
		{
			if (ImGui::MenuItem(ICON_AETOBJPIC "  Image")) { aetLayer.AddNewObject(AetObjType::Pic, "null.pic"); }
			if (ImGui::MenuItem(ICON_AETOBJEFF "  Layer")) { aetLayer.AddNewObject(AetObjType::Eff, "null_eff"); }
			if (ImGui::MenuItem(ICON_AETOBJAIF "  Sound Effect")) { aetLayer.AddNewObject(AetObjType::Aif, "null.aif"); }
			ImGui::EndMenu();
		}

		if (ImGui::MenuItem(ICON_MOVEUP "  Move Up")) {}
		if (ImGui::MenuItem(ICON_MOVEDOWN "  Move Down")) {}
		if (ImGui::MenuItem(ICON_DELETE "  Delete Layer")) { }

		return false;
	}

	bool AetTreeView::DrawAetObjContextMenu(AetLayer& aetLayer, AetObj& aetObj)
	{
		ImGui::Text("%s  %s", GetObjTypeIcon(aetObj.Type), aetObj.GetName().c_str());
		ImGui::Separator();

		// TODO:
		if (ImGui::MenuItem(ICON_MOVEUP "  Move Up")) {}
		if (ImGui::MenuItem(ICON_MOVEDOWN "  Move Down")) {}
		if (ImGui::MenuItem(ICON_DELETE "  Delete Object")) { objToDelete = { &aetLayer, &aetObj }; }

		return false;
	}

	const char* AetTreeView::GetDebugObjectName()
	{
		// return nullptr;
		return "target_slide18_r";
	}
}
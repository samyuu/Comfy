#include "AetTreeView.h"
#include "AetIcons.h"
#include "ImGui/imgui_extensions.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags SelectableTreeNodeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_OpenOnArrow;
	constexpr ImGuiTreeNodeFlags HeaderTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | SelectableTreeNodeFlags;

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

		if (ImGui::WideTreeNodeEx((void*)aetSet, HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str()))
		{
			if (ImGui::IsItemClicked())
				SetSelectedItem(aetSet);

			if (openLayers.size() != aetSet->size())
				openLayers.resize(aetSet->size());

			for (auto& aet : *aetSet)
				DrawTreeViewAet(aet);

			ImGui::TreePop();
		}
		else
		{
			ResetSelectedItem();
		}

		return true;
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
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			if (ImGui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::IsItemClicked())
					ResetSelectedItem();

				if (openLayers[aet.GetThisIndex()].size() != aet.AetLayers.size())
					openLayers[aet.GetThisIndex()].resize(aet.AetLayers.size());

				for (auto& aetLayer : aet.AetLayers)
					DrawTreeViewLayer(aet, aetLayer);

				ImGui::TreePop();
			}

			if (ImGui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				if (ImGui::IsItemClicked())
					ResetSelectedItem();

				for (int32_t i = 0; i < aet.AetRegions.size(); i++)
				{
					DrawTreeViewRegion(aet, aet.AetRegions[i], i);
				}
				ImGui::TreePop();
			}

			ImGui::PopStyleVar();
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

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

		bool isRoot = (&aetLayer == &aet.AetLayers.back());
		bool aetLayerNodeWasOpen = openLayers[aet.GetThisIndex()][aetLayer.GetThisIndex()];

		bool aetLayerNodeOpen = ImGui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags,
			isRoot ? "%s  Root" : "%s  Layer %d (%s)",
			aetLayerNodeWasOpen ? ICON_AETLAYER_OPEN : ICON_AETLAYER,
			aetLayer.GetThisIndex(),
			aetLayer.CommaSeparatedNames.c_str());

		openLayers[aet.GetThisIndex()][aetLayer.GetThisIndex()] = aetLayerNodeOpen;

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PopStyleColor();

		if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
			SetSelectedItem(&aet, &aetLayer);

		ImGui::OpenPopupOnItemClick(aetLayerContextMenuID, 1);

		bool openAddAetObjPopup = false;
		if (ImGui::BeginPopupContextWindow(aetLayerContextMenuID))
		{
			openAddAetObjPopup = AddAetObjContextMenu(aetLayer);
			ImGui::EndPopup();
		}

		if (openAddAetObjPopup)
			ImGui::OpenPopup(addAetObjPopupID);

		if (ImGui::BeginPopupModal(addAetObjPopupID, NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			AddAetObjPopup(aetLayer);
			ImGui::EndPopup();
		}

		if (aetLayerNodeOpen)
		{
			for (auto& aetObj : aetLayer)
				DrawTreeViewObj(aet, aetObj);

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewObj(Aet& aet, AetObj & aetObj)
	{
		ImGui::PushID((void*)&aetObj);
		{
			bool isSelected = &aetObj == selected.AetObj || &aetObj == hovered.AetObj;

			if (ImGui::SmallButton(aetObj.TypeFlag & AetTypeFlags_Visible ? ICON_VISIBLE : ICON_INVISIBLE, ImVec2(26, 0)))
				aetObj.TypeFlag ^= AetTypeFlags_Visible;
			ImGui::SameLine();

			sprintf_s(objNameBuffer, "%s  %s", GetObjTypeIcon(aetObj.Type), aetObj.Name.c_str());
			if (ImGui::Selectable(objNameBuffer, isSelected))
				SetSelectedItem(&aet, &aetObj);

			if (aetObj.Type == AetObjType::Eff && (ImGui::IsItemHovered() || &aetObj == selected.AetObj))
				hovered.SetItem(aetObj.GetLayer());
		}
		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewRegion(Aet& aet, AetRegion& region, int32_t index)
	{
		ImGui::PushID(&region);

		if (region.Sprites.size() >= 1)
		{
			sprintf_s(regionNameBuffer,
				ICON_AETREGION "  Region %d (%s)",
				index,
				region.Sprites.front().Name.c_str());
		}
		else
		{
			sprintf_s(regionNameBuffer,
				ICON_AETREGION "  Region %d (%dx%d)",
				index,
				region.Width, region.Height);
		}

		bool isSelected = &region == selected.AetRegion;

		if (ImGui::Selectable(regionNameBuffer, isSelected))
			SetSelectedItem(&aet, &region);

		ImGui::PopID();
	}

	bool AetTreeView::AddAetObjContextMenu(AetLayer& aetLayer)
	{
		bool openAddAetObjPopup = ImGui::MenuItem("Add new AetObj...");
		if (ImGui::MenuItem("Move Up")) {}
		if (ImGui::MenuItem("Move Down")) {}
		if (ImGui::MenuItem("Delete...")) {}

		return openAddAetObjPopup;
	}

	void AetTreeView::AddAetObjPopup(AetLayer& aetLayer)
	{
		if (ImGui::Combo("Obj Type", &newObjTypeIndex, AetObj::TypeNames.data(), AetObj::TypeNames.size()))
		{
			// TODO: automatically append .pic / .aif
		}

		ImGui::InputText("AetObj Name", newObjNameBuffer, sizeof(newObjNameBuffer));

		if (ImGui::Button("OK", ImVec2(124, 0)))
		{
			// TODO:
			// aetLayer.Objects.emplace(aetLayer.begin());
			// AetObj* newObj = &aetLayer.Objects.front();
			// 
			// newObj->Name = std::string(newObjNameBuffer);
			// newObj->Type = (AetObjType)newObjTypeIndex;
			// newObj->PlaybackSpeed = 1.0f;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(124, 0)))
		{
			ImGui::CloseCurrentPopup();
		}
	}
}
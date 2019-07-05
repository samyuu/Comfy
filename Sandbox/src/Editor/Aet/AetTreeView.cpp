#include "AetTreeView.h"
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
		hovered = { AetSelectionType::None, nullptr };

		if (ImGui::WideTreeNodeEx((void*)aetSet, HeaderTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str()))
		{
			//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			for (auto& aetLyo : aetSet->AetLyos)
				DrawTreeViewLyo(aetLyo);

			//ImGui::PopStyleVar();
			ImGui::TreePop();
		}
		else
		{
			ResetSelectedItem();
		}

		return true;
	}

	void AetTreeView::DrawTreeViewLyo(AetLyo& aetLyo)
	{
		ImGuiTreeNodeFlags lyoNodeFlags = HeaderTreeNodeFlags;
		if (&aetLyo == selected.AetLyo || &aetLyo == lastHovered.AetLyo)
			lyoNodeFlags |= ImGuiTreeNodeFlags_Selected;

		bool aetLyoNodeOpen = ImGui::WideTreeNodeEx((void*)&aetLyo, lyoNodeFlags, "%s", aetLyo.Name.c_str());

		if (ImGui::IsItemClicked())
			SetSelectedItem(&aetLyo, &aetLyo);

		if (aetLyoNodeOpen)
		{
			if (ImGui::WideTreeNodeEx("Layers", ImGuiTreeNodeFlags_DefaultOpen))
			{
				// TODO: alternate background color (draw own background quad (?))
				//auto textColorAlt = ImGui::GetStyleColorVec4(ImGuiCol_Text); textColorAlt.w *= .5f;

				int32_t layerIndex = 0;
				for (auto& aetLayer : aetLyo.AetLayers)
				{
					aetLayer.Index = layerIndex++;

					//if (layerIndex % 2 == 0)
					//	ImGui::PushStyleColor(ImGuiCol_Text, textColorAlt);
					
					DrawTreeViewLayer(aetLyo, aetLayer);
					
					//if (layerIndex % 2 == 0)
					//	ImGui::PopStyleColor();
				}
				ImGui::TreePop();
			}

			if (ImGui::WideTreeNode("Regions"))
			{
				for (int32_t i = 0; i < aetLyo.AetRegions.size(); i++)
					DrawTreeViewRegion(aetLyo, aetLyo.AetRegions[i], i);
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}

	void AetTreeView::DrawTreeViewLayer(AetLyo& aetLyo, AetLayer& aetLayer)
	{
		ImGui::PushID(&aetLayer);

		ImGuiTreeNodeFlags layerNodeFlags = SelectableTreeNodeFlags;
		if (&aetLayer == selected.AetLayer)
			layerNodeFlags |= ImGuiTreeNodeFlags_Selected;

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

		AetLayer* rootLayer = &aetLyo.AetLayers.back();
		bool aetLayerNodeOpen = ImGui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags, (&aetLayer == rootLayer) ? "Root" : "Layer %d (%s)", aetLayer.Index, aetLayer.CommaSeparatedNames.c_str());

		if (&aetLayer == lastHovered.AetLayer)
			ImGui::PopStyleColor();

		if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
			SetSelectedItem(&aetLyo, &aetLayer);

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
			for (auto& aetObj : aetLayer.Objects)
			{
				ImGui::PushID((void*)&aetObj);

				bool isSelected = &aetObj == selected.AetObj || &aetObj == hovered.AetObj;

				if (ImGui::Selectable(aetObj.Name.c_str(), isSelected))
					SetSelectedItem(&aetLyo, &aetObj);

				if (aetObj.Type == AetObjType_Eff && (ImGui::IsItemHovered() || &aetObj == selected.AetObj))
					hovered = { AetSelectionType::AetLayer, aetObj.ReferencedLayer };

				ImGui::PopID();
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewRegion(AetLyo& aetLyo, AetRegion& region, int32_t index)
	{
		ImGui::PushID(&region);

		char regionNameBuffer[32];
		sprintf_s(regionNameBuffer, "Region %d", index);

		bool isSelected = &region == selected.AetRegion;

		if (ImGui::Selectable(regionNameBuffer, isSelected))
			SetSelectedItem(&aetLyo, &region);

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
			aetLayer.Objects.emplace_front();

			AetObj* newObj = &aetLayer.Objects.front();
			newObj->Name = std::string(newObjNameBuffer);
			newObj->Type = (AetObjType)newObjTypeIndex;
			newObj->PlaybackSpeed = 1.0f;

			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		
		if (ImGui::Button("Cancel", ImVec2(124, 0))) 
		{ 
			ImGui::CloseCurrentPopup(); 
		}
	}
}
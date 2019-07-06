#include "AetTreeView.h"
#include "ImGui/imgui_extensions.h"
#include <FontIcons.h>

#define ICON_AETLAYERS		ICON_FA_LAYER_GROUP
#define ICON_AETLAYER		ICON_FA_FOLDER
#define ICON_AETLAYER_OPEN	ICON_FA_FOLDER_OPEN
#define ICON_AETREGIONS		ICON_FA_IMAGES
#define ICON_AETREGION		ICON_FA_IMAGE
#define ICON_AETOBJNOP		ICON_FA_QUESTION_CIRCLE
#define ICON_AETOBJPIC		ICON_FA_DICE_D6
#define ICON_AETOBJAIF		ICON_FA_VOLUME_UP
#define ICON_AETOBJEFF		ICON_FA_CUBES
#define ICON_VISIBLE		ICON_FA_EYE
#define ICON_INVISIBLE		ICON_FA_EYE_SLASH

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
			for (auto& aetLyo : aetSet->AetLyos)
				DrawTreeViewLyo(aetLyo);

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
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			if (ImGui::WideTreeNodeEx(ICON_AETLAYERS "  Layers", SelectableTreeNodeFlags | ImGuiTreeNodeFlags_DefaultOpen))
			{
				// TODO: alternate background color (draw own background quad (?))
				//auto textColorAlt = ImGui::GetStyleColorVec4(ImGuiCol_Text); textColorAlt.w *= .5f;

				if (openLayers.size() != aetLyo.AetLayers.size())
					openLayers.resize(aetLyo.AetLayers.size());

				int32_t layerIndex = 0;
				for (auto& aetLayer : aetLyo.AetLayers)
				{
					aetLayer.Index = layerIndex++;

					DrawTreeViewLayer(aetLyo, aetLayer);
				}
				ImGui::TreePop();
			}

			if (ImGui::WideTreeNodeEx(ICON_AETREGIONS "  Regions", SelectableTreeNodeFlags))
			{
				for (int32_t i = 0; i < aetLyo.AetRegions.size(); i++)
					DrawTreeViewRegion(aetLyo, aetLyo.AetRegions[i], i);
				ImGui::TreePop();
			}

			ImGui::PopStyleVar();
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
		bool aetLayerNodeOpen = ImGui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags, (&aetLayer == rootLayer)
			? "%s  Root"
			: "%s  Layer %d (%s)",
			openLayers[aetLayer.Index] ? ICON_AETLAYER_OPEN : ICON_AETLAYER, aetLayer.Index, aetLayer.CommaSeparatedNames.c_str());

		openLayers[aetLayer.Index] = aetLayerNodeOpen;

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
				DrawTreeViewObj(aetLyo, aetObj);
			}
			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewObj(AetLyo & aetLyo, AetObj & aetObj)
	{
		char objNameBuffer[255];

		ImGui::PushID((void*)&aetObj);
		{
			bool isSelected = &aetObj == selected.AetObj || &aetObj == hovered.AetObj;

			sprintf_s(objNameBuffer, "%s  %s", GetTypeIcon(aetObj.Type), aetObj.Name.c_str());

			{
				bool visible = (aetObj.TypeFlag & AetTypeFlags_Visible);
				if (ImGui::SmallButton(visible ? ICON_VISIBLE : ICON_INVISIBLE, ImVec2(26, 0)))
					aetObj.TypeFlag ^= AetTypeFlags_Visible;
				ImGui::SameLine();
			}
			{
				if (ImGui::Selectable(objNameBuffer, isSelected))
					SetSelectedItem(&aetLyo, &aetObj);
			}

			if (aetObj.Type == AetObjType_Eff && (ImGui::IsItemHovered() || &aetObj == selected.AetObj))
				hovered = { AetSelectionType::AetLayer, aetObj.ReferencedLayer };
		}
		ImGui::PopID();
	}

	void AetTreeView::DrawTreeViewRegion(AetLyo& aetLyo, AetRegion& region, int32_t index)
	{
		ImGui::PushID(&region);

		char regionNameBuffer[255];
		sprintf_s(regionNameBuffer,
			ICON_AETREGION "  Region %d (%s)",
			index,
			region.Sprites.size() == 1 ? region.Sprites.front().Name.c_str() : "...");

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

	const char* AetTreeView::GetTypeIcon(AetObjType type) const
	{
		switch (type)
		{
		case AetObjType_Pic:
			return ICON_AETOBJPIC;
		case AetObjType_Aif:
			return ICON_AETOBJPIC;
		case AetObjType_Eff:
			return ICON_AETOBJEFF;
		case AetObjType_Nop:
		default:
			return ICON_AETOBJNOP;
		}
	}
}
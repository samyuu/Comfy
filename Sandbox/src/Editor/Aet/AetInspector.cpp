#include "AetInspector.h"
#include "AetIcons.h"
#include "ImGui/imgui_extensions.h"

namespace Editor
{
	AetInspector::AetInspector()
	{
	}

	AetInspector::~AetInspector()
	{
	}

	void AetInspector::Initialize()
	{
	}

	bool AetInspector::DrawGui(Aet* aet, const AetItemTypePtr& selected)
	{
		if (lastSelectedItem.VoidPointer != selected.VoidPointer)
			newParentObjLayerIndex = -1;

		lastSelectedItem = selected;

		if (selected.VoidPointer == nullptr)
			return false;

		switch (selected.Type())
		{
		case AetSelectionType::AetSet:
			DrawInspectorAetSet(selected.AetSet);
			break;
		case AetSelectionType::Aet:
			DrawInspectorAet(selected.Aet);
			break;
		case AetSelectionType::AetLayer:
			DrawInspectorAetLayer(aet, selected.AetLayer);
			break;
		case AetSelectionType::AetObj:
			DrawInspectorAetObj(aet, selected.AetObj);
			break;
		case AetSelectionType::AetRegion:
			DrawInspectorAetRegion(aet, selected.AetRegion);
			break;
		default:
			break;
		}

		return true;
	}

	void AetInspector::DrawInspectorAetSet(AetSet* aetSet)
	{
		ImGui::Text("AetSet:");
		{
			if (ImGui::WideTreeNodeEx(ICON_NAMES "  Aets:", ImGuiTreeNodeFlags_DefaultOpen))
			{
				for (auto& aet : *aetSet)
					ImGui::BulletText(aet.Name.c_str());

				ImGui::TreePop();
			}
		}
	}

	void AetInspector::DrawInspectorAet(Aet* aet)
	{
		ImGui::Text("Aet:");
		{
			strcpy_s(aetNameBuffer, aet->Name.c_str());

			if (ImGui::InputText("Name##Aet", aetNameBuffer, sizeof(aetNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				aet->Name = std::string(aetNameBuffer);

			ImGui::InputFloat("Start Frame", &aet->FrameStart, 1.0f, 10.0f);
			ImGui::InputFloat("Duration", &aet->FrameDuration, 1.0f, 10.0f);
			if (ImGui::InputFloat("Frame Rate", &aet->FrameRate, 1.0f, 10.0f))
				aet->FrameRate = glm::clamp(aet->FrameRate, 1.0f, 1000.0f);
			ImGui::InputInt2("Resolution", &aet->Width);

			ImVec4 color = ImGui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (ImGui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
				aet->BackgroundColor = ImGui::ColorConvertFloat4ToU32(color);
		}
	}

	void AetInspector::DrawInspectorAetLayer(Aet* aet, AetLayer* aetLayer)
	{
		ImGui::Text("AetLayer:");

		if (ImGui::WideTreeNodeEx(ICON_NAMES "  Given Names:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& name : aetLayer->GetGivenNames())
				ImGui::BulletText(name.c_str());

			ImGui::TreePop();
		}

		if (ImGui::WideTreeNodeEx(ICON_AETLAYER "  Objects:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& aetObj : *aetLayer)
				ImGui::BulletText("%s  %s", GetObjTypeIcon(aetObj.Type), aetObj.GetName().c_str());

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(Aet* aet, AetObj* aetObj, AetLayer* aetLayer)
	{
		if (ImGui::TreeNodeEx(ICON_AETLAYERS "  Layer Data", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (aetLayer != nullptr)
				sprintf_s(layerDataNameBuffer, "Layer %d (%s)", aetLayer->GetThisIndex(), aetLayer->GetCommaSeparatedNames());

			if (ImGui::BeginCombo("Layer", aetLayer == nullptr ? "nullptr" : layerDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (ImGui::Selectable("nullptr", aetLayer == nullptr))
					aetObj->SetLayer(nullptr);

				for (auto& layer : aet->AetLayers)
				{
					if (&aet->AetLayers.back() == &layer)
						break;

					ImGui::PushID(&layer);

					bool isSelected = (aetLayer == &layer);
					sprintf_s(layerDataNameBuffer, "Layer %d (%s)", layer.GetThisIndex(), layer.GetCommaSeparatedNames());

					if (ImGui::Selectable(layerDataNameBuffer, isSelected))
						aetObj->SetLayer(&layer);

					if (isSelected)
						ImGui::SetItemDefaultFocus();
					ImGui::PopID();
				}

				ImGui::EndCombo();
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObj(Aet* aet, AetObj* aetObj)
	{
		ImGui::Text("AetObj:");

		{
			strcpy_s(aetObjNameBuffer, aetObj->GetName().c_str());

			if (ImGui::InputText("Name##AetObj", aetObjNameBuffer, sizeof(aetObjNameBuffer), ImGuiInputTextFlags_None /*ImGuiInputTextFlags_EnterReturnsTrue*/))
			{
				aetObj->SetName(aetObjNameBuffer);
			}

			int objTypeIndex = static_cast<int>(aetObj->Type);
			if (ImGui::Combo("Obj Type", &objTypeIndex, AetObj::TypeNames.data(), static_cast<int>(AetObj::TypeNames.size())))
				aetObj->Type = static_cast<AetObjType>(objTypeIndex);

			ImGui::InputFloat("Loop Start", &aetObj->LoopStart, 1.0f, 10.0f);
			ImGui::InputFloat("Loop End", &aetObj->LoopEnd, 1.0f, 10.0f);
			ImGui::InputFloat("Start Frame", &aetObj->StartFrame, 1.0f, 10.0f);
			ImGui::InputFloat("Playback Speed", &aetObj->PlaybackSpeed, 0.1f, 1.0f);
		}

		if ((aetObj->Type == AetObjType::Pic))
			DrawInspectorRegionData(aet, aetObj, aetObj->GetRegion());

		if ((aetObj->Type == AetObjType::Eff))
			DrawInspectorLayerData(aet, aetObj, aetObj->GetLayer());

		if ((aetObj->Type == AetObjType::Pic || aetObj->Type == AetObjType::Eff))
			DrawInspectorAnimationData(aetObj->AnimationData.get());

		DrawInspectorAetObjMarkers(&aetObj->Markers);
		DrawInspectorAetObjParent(aet, aetObj);
	}

	void AetInspector::DrawInspectorRegionData(Aet* aet, AetObj* aetObj, AetRegion* aetRegion)
	{
		if (ImGui::TreeNodeEx(ICON_AETREGIONS "  Region Data", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (aetRegion != nullptr)
			{
				AetSprite* frontSprite = aetRegion->GetFrontSprite();

				if (frontSprite == nullptr)
					sprintf_s(regionDataNameBuffer, "Dynamic Region (%dx%d)", aetRegion->Width, aetRegion->Height);
				else
					strcpy_s(regionDataNameBuffer, frontSprite->Name.c_str());
			}

			if (ImGui::BeginCombo("Region", aetRegion == nullptr ? "nullptr" : regionDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (ImGui::Selectable("nullptr", aetRegion == nullptr))
					aetObj->SetRegion(nullptr);

				int32_t regionIndex = 0;
				for (auto& region : aet->AetRegions)
				{
					ImGui::PushID(&region);

					bool isSelected = (aetRegion == &region);

					AetSprite* frontSprite = region.GetFrontSprite();
					if (frontSprite == nullptr)
						sprintf_s(regionDataNameBuffer, "Region %d (%dx%d)", regionIndex, region.Width, region.Height);

					if (ImGui::Selectable(frontSprite == nullptr ? regionDataNameBuffer : frontSprite->Name.c_str(), isSelected))
						aetObj->SetRegion(&region);

					if (isSelected)
						ImGui::SetItemDefaultFocus();
					ImGui::PopID();
					regionIndex++;
				}

				ImGui::EndCombo();
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(AnimationData* animationData)
	{
		if (ImGui::TreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			if (ImGui::WideTreeNode("Properties"))
			{
				if (animationData != nullptr)
				{
					DrawInspectorKeyFrameProperties(&animationData->Properties);
				}
				else
				{
					ImGui::BulletText("none");
				}
				ImGui::TreePop();
			}

			if (ImGui::WideTreeNode("Perspective Properties"))
			{
				if (animationData->PerspectiveProperties != nullptr)
				{
					DrawInspectorKeyFrameProperties(animationData->PerspectiveProperties.get());
				}
				else
				{
					ImGui::BulletText("none");
				}
				ImGui::TreePop();
			}

			const char* blendModeNames = "Alpha\0None\0Additive\0DstColorZero\0SrcAlphaOneMinusSrcColor\0Transparent";

			int32_t blendMode = static_cast<int32_t>(animationData->BlendMode) - 3;
			if (ImGui::Combo("Blend Mode", &blendMode, blendModeNames))
				animationData->BlendMode = static_cast<AetBlendMode>(blendMode + 3);

			ImGui::Checkbox("Use Texture Mask", &animationData->UseTextureMask);

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorKeyFrameProperties(KeyFrameProperties* properties)
	{
		for (size_t i = 0; i < properties->size(); i++)
			DrawInspectorKeyFrames(KeyFrameProperties::PropertyNames[i], &properties->at(i));
	}

	void AetInspector::DrawInspectorKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames)
	{
		if (ImGui::WideTreeNode(name))
		{
			if (keyFrames->size() == 1)
			{
				ImGui::PushID((void*)keyFrames);
				ImGui::DragFloat("Value", &keyFrames->front().Value, 0.1f, 1.0f);
				ImGui::PopID();
			}
			else
			{
				for (KeyFrame& keyFrame : *keyFrames)
				{
					ImGui::PushID((void*)&keyFrame.Frame);
					ImGui::DragFloat3("F-V-I", &keyFrame.Frame);
					ImGui::PopID();
				}
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObjMarkers(std::vector<Marker>* markers)
	{
		if (ImGui::TreeNodeEx(ICON_MARKERS "  Markers", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (int i = 0; i < markers->size(); i++)
			{
				Marker& marker = markers->at(i);

				ImGui::PushID((void*)&marker);
				bool open = ImGui::WideTreeNode("##AetInspectorMarker", "Frame: %.2f  :  %s", marker.Frame, marker.Name.c_str());

				ImGui::ItemContextMenu("AddMarkerContextMenu##AetInspector", [&markers, &i]()
				{
					if (ImGui::MenuItem(ICON_MOVEUP "  Move Up", nullptr, nullptr, i > 0))
						std::iter_swap(markers->begin() + i, markers->begin() + i - 1);

					if (ImGui::MenuItem(ICON_MOVEDOWN "  Move Down", nullptr, nullptr, i < markers->size() - 1))
						std::iter_swap(markers->begin() + i, markers->begin() + i + 1);

					if (ImGui::MenuItem(ICON_DELETE "  Delete"))
					{
						markers->erase(markers->begin() + i);
						i--;
					}
				});

				if (open)
				{
					ImGui::InputFloat("Frame", &marker.Frame, 1.0f, 10.0f);
					strcpy_s(markerNameBuffer, marker.Name.c_str());

					if (ImGui::InputText("Name", markerNameBuffer, sizeof(markerNameBuffer)))
						marker.Name = std::string(markerNameBuffer);

					ImGui::TreePop();
				}
				ImGui::PopID();
			}

			if (ImGui::Button("Add Marker", ImVec2(ImGui::GetWindowWidth(), 0)))
			{
				char newMarkerBuffer[32];
				sprintf_s(newMarkerBuffer, "marker_%02zd", markers->size());
				markers->emplace_back(0.0f, newMarkerBuffer);
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObjParent(Aet* aet, AetObj* aetObj)
	{
		if (ImGui::TreeNodeEx(ICON_PARENT "  Parent", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			AetObj* parentObj = aetObj->GetParentObj();
			AetLayer* parentObjLayer = aetObj->GetParentObjLayer();

			if (parentObj != nullptr)
				newParentObjLayerIndex = aetObj->GetParentObjLayerIndex();

			if (newParentObjLayerIndex >= 0)
			{
				parentObjLayer = &aet->AetLayers[newParentObjLayerIndex];
				sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", parentObjLayer->GetThisIndex(), parentObjLayer->GetCommaSeparatedNames());
			}
			else
			{
				strcpy_s(parentObjDataNameBuffer, "nullptr");
			}

			if (ImGui::BeginCombo("Parent Layer", parentObjDataNameBuffer, ImGuiComboFlags_HeightLarge))
			{
				if (ImGui::Selectable("nullptr", parentObjLayer == nullptr))
				{
					aetObj->SetParentObj(nullptr);
					newParentObjLayerIndex = -1;
				}

				for (int32_t layerIndex = 0; layerIndex < aet->AetLayers.size(); layerIndex++)
				{
					auto& layer = aet->AetLayers[layerIndex];

					bool isSelected = (layerIndex == newParentObjLayerIndex);
					sprintf_s(parentObjDataNameBuffer, "Layer %d (%s)", layer.GetThisIndex(), layer.GetCommaSeparatedNames());

					ImGui::PushID(&layer);
					if (ImGui::Selectable(parentObjDataNameBuffer, isSelected))
					{
						aetObj->SetParentObj(nullptr);
						newParentObjLayerIndex = layerIndex;
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
					ImGui::PopID();
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("Parent Object", parentObj == nullptr ? "nullptr" : parentObj->GetName().c_str(), ImGuiComboFlags_HeightLarge))
			{
				if (ImGui::Selectable("nullptr", parentObj == nullptr))
					aetObj->SetParentObj(nullptr);

				if (parentObjLayer != nullptr)
				{
					for (int32_t objIndex = 0; objIndex < parentObjLayer->size(); objIndex++)
					{
						auto& obj = parentObjLayer->at(objIndex);
						bool isSelected = (&obj == parentObj);

						ImGui::PushID(&obj);
						if (ImGui::Selectable(obj.GetName().c_str(), isSelected))
							aetObj->SetParentObj(&obj);

						if (isSelected)
							ImGui::SetItemDefaultFocus();
						ImGui::PopID();
					}
				}
				ImGui::EndCombo();

			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetRegion(Aet* aet, AetRegion* aetRegion)
	{
		ImGui::Text("AetRegion:");

		ImGui::InputScalarN("Dimensions", ImGuiDataType_S16, &aetRegion->Width, 2);

		ImVec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
		if (ImGui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			aetRegion->Color = ImGui::ColorConvertFloat4ToU32(color);

		if (ImGui::TreeNodeEx("Sprites:", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& sprite : aetRegion->GetSprites())
			{
				sprintf_s(spriteNameBuffer, ICON_AETREGION "  %s", sprite.Name.c_str());
				ImGui::Selectable(spriteNameBuffer);
			}

			ImGui::TreePop();
		}
	}
}
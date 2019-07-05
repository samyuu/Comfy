#include "AetInspector.h"
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

	bool AetInspector::DrawGui(AetSet* aetSet, const AetItemTypePtr& selected)
	{
		ImGui::Text("Selection:");
		ImGui::Separator();

		if (selected.ItemPtr == nullptr)
		{
			ImGui::Text("nullptr");
			return false;
		}

		switch (selected.Type)
		{
		case AetSelectionType::AetObj:
			DrawInspectorAetObj(aetSet, selected.AetObj);
			break;
		case AetSelectionType::AetLayer:
			DrawInspectorAetLayer(aetSet, selected.AetLayer);
			break;
		case AetSelectionType::AetLyo:
			DrawInspectorAetLyo(aetSet, selected.AetLyo);
			break;
		case AetSelectionType::AetRegion:
			DrawInspectorAetRegion(aetSet, selected.AetRegion);
			break;
		default:
			break;
		}

		return true;
	}

	void AetInspector::DrawInspectorAetObj(AetSet* aetSet, AetObj* aetObj)
	{
		ImGui::Text("AetObj: %s", aetObj->Name.c_str());

		if (ImGui::WideTreeNodeEx("Object Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static char aetObjNameBuffer[255];
			strcpy_s(aetObjNameBuffer, aetObj->Name.c_str());

			if (ImGui::InputText("Name##AetObj", aetObjNameBuffer, sizeof(aetObjNameBuffer), ImGuiInputTextFlags_None /*ImGuiInputTextFlags_EnterReturnsTrue*/))
			{
				aetObj->Name = std::string(aetObjNameBuffer);
				aetSet->UpdateLayerNames();
			}

			int objTypeIndex = aetObj->Type;
			if (ImGui::Combo("Obj Type", &objTypeIndex, AetObj::TypeNames.data(), AetObj::TypeNames.size()))
				aetObj->Type = (AetObjType)objTypeIndex;

			ImGui::InputFloat("Loop Start", &aetObj->LoopStart);
			ImGui::InputFloat("Loop End", &aetObj->LoopEnd);
			ImGui::InputFloat("Start Frame", &aetObj->StartFrame);

			uint32_t flags = aetObj->TypeFlag;
			if (ImGui::CheckboxFlags("Visible", &flags, AetTypeFlags_Visible))
				aetObj->TypeFlag = flags;

			ImGui::TreePop();
		}

		if ((aetObj->Type == AetObjType_Pic))
			DrawInspectorRegionData(aetObj->ReferencedRegion);

		if ((aetObj->Type == AetObjType_Eff))
			DrawInspectorLayerData(aetObj->ReferencedLayer);

		if ((aetObj->Type == AetObjType_Pic || aetObj->Type == AetObjType_Eff))
			DrawInspectorAnimationData(&aetObj->AnimationData);
	}

	void AetInspector::DrawInspectorRegionData(AetRegion* aetRegion)
	{
		if (ImGui::WideTreeNode("Region Data"))
		{
			if (aetRegion != nullptr)
			{
				ImVec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
				if (ImGui::ColorEdit4("##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
					aetRegion->Color = ImGui::ColorConvertFloat4ToU32(color);

				ImGui::InputScalarN("Dimensions", ImGuiDataType_S16, &aetRegion->Width, 2);

				if (ImGui::WideTreeNodeEx("Sprites", ImGuiTreeNodeFlags_DefaultOpen))
				{
					for (auto& sprite : aetRegion->Sprites)
						ImGui::Selectable(sprite.Name.c_str());

					ImGui::TreePop();
				}
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(AetLayer* aetLayer)
	{
		if (ImGui::WideTreeNode("Layer Data"))
		{
			if (aetLayer != nullptr)
			{
				ImGui::Text("Layer %d (%s)", aetLayer->Index, aetLayer->CommaSeparatedNames.c_str());
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(AnimationData* animationData)
	{
		if (ImGui::WideTreeNode("Animation Data"))
		{
			ImGui::Checkbox("Use Texture Mask", &animationData->UseTextureMask);

			if (ImGui::WideTreeNode("Properties"))
			{
				if (animationData->Properties != nullptr)
					DrawInspectorKeyFrameProperties(animationData->Properties.get());
				else ImGui::Text("nullptr");
				ImGui::TreePop();
			}

			if (ImGui::WideTreeNode("Perspective Properties"))
			{
				if (animationData->PerspectiveProperties != nullptr)
					DrawInspectorKeyFrameProperties(animationData->PerspectiveProperties.get());
				else ImGui::Text("nullptr");
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorKeyFrameProperties(KeyFrameProperties* properties)
	{
		size_t keyFrameIndex = 0;
		for (auto keyFrames = &properties->OriginX; keyFrames <= &properties->Opacity; keyFrames++)
			DrawInspectorKeyFrames(KeyFrameProperties::PropertyNames[keyFrameIndex++], keyFrames);
	}

	void AetInspector::DrawInspectorKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames)
	{
		if (ImGui::WideTreeNode(name))
		{
			for (KeyFrame& keyFrame : *keyFrames)
			{
				ImGui::PushID((void*)&keyFrame.Frame);
				ImGui::InputFloat3("F-V-I", &keyFrame.Frame);
				ImGui::PopID();
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetLayer(AetSet* aetSet, AetLayer* aetLayer)
	{
		ImGui::Text("AetLayer:");

		if (ImGui::WideTreeNodeEx("Names:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& name : aetLayer->Names)
				ImGui::BulletText(name.c_str());

			ImGui::TreePop();
		}

		if (ImGui::WideTreeNodeEx("Objects:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& aetObj : aetLayer->Objects)
				ImGui::BulletText(aetObj.Name.c_str());

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetLyo(AetSet* aetSet, AetLyo* aetLyo)
	{
		ImGui::Text("AetLyo: %s", aetLyo->Name.c_str());

		if (ImGui::WideTreeNodeEx("Lyo Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static char aetLyoNameBuffer[255];
			strcpy_s(aetLyoNameBuffer, aetLyo->Name.c_str());

			if (ImGui::InputText("Name##AetObj", aetLyoNameBuffer, sizeof(aetLyoNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				aetLyo->Name = std::string(aetLyoNameBuffer);

			ImGui::InputFloat("Frame Rate", &aetLyo->FrameRate);
			ImGui::InputFloat("Duration", &aetLyo->FrameDuration);
			ImGui::InputInt2("Resolution", &aetLyo->Width);

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetRegion(AetSet* aetSet, AetRegion* aetRegion)
	{
		ImGui::Text("AetRegion:");

		ImGui::InputScalarN("Dimensions", ImGuiDataType_S16, &aetRegion->Width, 2);

		ImVec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
		if (ImGui::ColorEdit4("##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			aetRegion->Color = ImGui::ColorConvertFloat4ToU32(color);

		if (ImGui::WideTreeNodeEx("Sprites:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& sprite : aetRegion->Sprites)
				ImGui::Selectable(sprite.Name.c_str());

			ImGui::TreePop();
		}
	}
}
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

	bool AetInspector::DrawGui(AetSet* aetSet, const AetItemTypePtr& selected)
	{
		if (ImGui::TreeNodeEx("Selection", ImGuiTreeNodeFlags_CollapsingHeader | ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Separator();

			if (selected.VoidPointer == nullptr)
			{
				ImGui::BulletText("<none>");
				return false;
			}

			switch (selected.Type())
			{
			case AetSelectionType::AetSet:
				DrawInspectorAetSet(aetSet);
				break;
			case AetSelectionType::Aet:
				DrawInspectorAet(aetSet, selected.Aet);
				break;
			case AetSelectionType::AetLayer:
				DrawInspectorAetLayer(aetSet, selected.AetLayer);
				break;
			case AetSelectionType::AetObj:
				DrawInspectorAetObj(aetSet, selected.AetObj);
				break;
			case AetSelectionType::AetRegion:
				DrawInspectorAetRegion(aetSet, selected.AetRegion);
				break;
			default:
				break;
			}
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

	void AetInspector::DrawInspectorAet(AetSet* aetSet, Aet* aet)
	{
		ImGui::Text("Aet:");
		{
			strcpy_s(aetNameBuffer, aet->Name.c_str());

			if (ImGui::InputText("Name##Aet", aetNameBuffer, sizeof(aetNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				aet->Name = std::string(aetNameBuffer);

			ImGui::InputFloat("Frame Rate", &aet->FrameRate);
			ImGui::InputFloat("Duration", &aet->FrameDuration);
			ImGui::InputInt2("Resolution", &aet->Width);

			ImVec4 color = ImGui::ColorConvertU32ToFloat4(aet->BackgroundColor);
			if (ImGui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
				aet->BackgroundColor = ImGui::ColorConvertFloat4ToU32(color);
		}
	}

	void AetInspector::DrawInspectorAetLayer(AetSet* aetSet, AetLayer* aetLayer)
	{
		ImGui::Text("AetLayer:");

		if (ImGui::WideTreeNodeEx(ICON_NAMES "  Names:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& name : aetLayer->Names)
				ImGui::BulletText(name.c_str());

			ImGui::TreePop();
		}

		if (ImGui::WideTreeNodeEx(ICON_AETLAYER "  Objects:", ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& aetObj : *aetLayer)
				ImGui::BulletText("%s  %s", GetObjTypeIcon(aetObj.Type), aetObj.Name.c_str());

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorLayerData(AetLayer* aetLayer)
	{
		if (ImGui::TreeNodeEx(ICON_AETLAYERS "  Layer Data", ImGuiTreeNodeFlags_Framed))
		{
			if (aetLayer != nullptr)
			{
				ImGui::BulletText(ICON_AETLAYER "  Layer %d (%s)", aetLayer->GetThisIndex(), aetLayer->CommaSeparatedNames.c_str());
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObj(AetSet* aetSet, AetObj* aetObj)
	{
		ImGui::Text("AetObj:");

		{
			strcpy_s(aetObjNameBuffer, aetObj->Name.c_str());

			if (ImGui::InputText("Name##AetObj", aetObjNameBuffer, sizeof(aetObjNameBuffer), ImGuiInputTextFlags_None /*ImGuiInputTextFlags_EnterReturnsTrue*/))
			{
				aetObj->Name = std::string(aetObjNameBuffer);
				aetSet->UpdateLayerNames();
			}

			int objTypeIndex = static_cast<int>(aetObj->Type);
			if (ImGui::Combo("Obj Type", &objTypeIndex, AetObj::TypeNames.data(), AetObj::TypeNames.size()))
				aetObj->Type = static_cast<AetObjType>(objTypeIndex);

			ImGui::InputFloat("Loop Start", &aetObj->LoopStart, 1.0f, 10.0f);
			ImGui::InputFloat("Loop End", &aetObj->LoopEnd, 1.0f, 10.0f);
			ImGui::InputFloat("Start Frame", &aetObj->StartFrame, 1.0f, 10.0f);
		}

		if ((aetObj->Type == AetObjType::Pic))
			DrawInspectorRegionData(aetObj->GetRegion());

		if ((aetObj->Type == AetObjType::Eff))
			DrawInspectorLayerData(aetObj->GetLayer());

		if ((aetObj->Type == AetObjType::Pic || aetObj->Type == AetObjType::Eff))
			DrawInspectorAnimationData(&aetObj->AnimationData);

		DrawInspectorAetObjParent(aetObj);
	}

	void AetInspector::DrawInspectorRegionData(AetRegion* aetRegion)
	{
		if (ImGui::TreeNodeEx(ICON_AETREGIONS "  Region Data", ImGuiTreeNodeFlags_Framed))
		{
			if (aetRegion != nullptr)
			{
				if (aetRegion->Sprites.size() < 1)
				{
					ImGui::BulletText("<%dx%d>", aetRegion->Width, aetRegion->Height);
				}
				else
				{
					for (auto& sprite : aetRegion->Sprites)
						ImGui::BulletText(sprite.Name.c_str());
				}
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAnimationData(AnimationData* animationData)
	{
		if (ImGui::TreeNodeEx(ICON_ANIMATIONDATA "  Animation Data", ImGuiTreeNodeFlags_Framed))
		{
			if (ImGui::WideTreeNode("Properties"))
			{
				if (animationData->Properties != nullptr)
				{
					DrawInspectorKeyFrameProperties(animationData->Properties.get());
				}
				else
				{
					ImGui::BulletText("<none>");
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
					ImGui::BulletText("<none>");
				}
				ImGui::TreePop();
			}

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
			for (KeyFrame& keyFrame : *keyFrames)
			{
				ImGui::PushID((void*)&keyFrame.Frame);
				ImGui::InputFloat3("F-V-I", &keyFrame.Frame);
				ImGui::PopID();
			}
			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetObjParent(AetObj* aetObj)
	{
		if (ImGui::TreeNodeEx(ICON_PARENT "  Parent", ImGuiTreeNodeFlags_Framed))
		{
			AetObj* parent = aetObj->GetParent();

			if (parent == nullptr)
			{
				ImGui::BulletText("<none>");
			}
			else
			{
				ImGui::BulletText("%s  %s", GetObjTypeIcon(parent->Type), parent->Name.c_str());
			}

			ImGui::TreePop();
		}
	}

	void AetInspector::DrawInspectorAetRegion(AetSet* aetSet, AetRegion* aetRegion)
	{
		ImGui::Text("AetRegion:");

		ImGui::InputScalarN("Dimensions", ImGuiDataType_S16, &aetRegion->Width, 2);

		ImVec4 color = ImGui::ColorConvertU32ToFloat4(aetRegion->Color);
		if (ImGui::ColorEdit3("Background##AetRegionColor", (float*)&color, ImGuiColorEditFlags_DisplayHex))
			aetRegion->Color = ImGui::ColorConvertFloat4ToU32(color);

		if (ImGui::TreeNodeEx("Sprites:", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
		{
			for (auto& sprite : aetRegion->Sprites)
			{
				sprintf_s(spriteNameBuffer, ICON_AETREGION "  %s", sprite.Name.c_str());
				ImGui::Selectable(spriteNameBuffer);
			}

			ImGui::TreePop();
		}
	}
}
#include "AetEditor.h"
#include "FileSystem/MemoryStream.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	AetEditor::~AetEditor()
	{
	}

	void AetEditor::Initialize()
	{
		OpenAetSet(testAetPath);

		treeView = std::make_unique<AetTreeView>(&activeAetLyo, &selected, &hovered, &lastHovered);
		treeView->Initialize();

		timeline = std::make_unique<AetTimeline>();
		timeline->InitializeTimelineGuiState();

		renderWindow = std::make_unique<AetRenderWindow>();
		renderWindow->Initialize();
	}

	void AetEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoBackground;

		if (ImGui::Begin("AetSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Tree View##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetTreeViewChild##AetEditor", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			treeView->DrawGui(aetSet.get());
			ImGui::EndChild();
		}
		ImGui::End();

		RenderWindowBase::PushWindowPadding();
		if (ImGui::Begin("Aet Render Window##AetEditor", nullptr, windowFlags))
		{
			renderWindow->SetAetLyo(activeAetLyo);
			renderWindow->SetAetObj(selected.Type == AetSelectionType::AetObj ? selected.AetObj : nullptr);
			renderWindow->DrawGui();
		}
		ImGui::End();
		RenderWindowBase::PopWindowPadding();

		if (ImGui::Begin("Aet Inspector##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetInspectorChild##AetEditor");
			DrawInspector();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Properties##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetPropertiesChild##AetEditor");
			DrawProperties();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Timeline##AetEditor", nullptr))
		{
			timeline->SetAetObj(selected.Type == AetSelectionType::AetObj ? selected.AetObj : nullptr);
			timeline->DrawTimelineGui();
		}
		ImGui::End();
	}

	const char* AetEditor::GetGuiName() const
	{
		return u8"Aet Editor";
	}

	ImGuiWindowFlags AetEditor::GetWindowFlags() const
	{
		return BaseWindow::GetNoWindowFlags();
	}

	void AetEditor::DrawInspectorAetObj(AetObj* aetObj)
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

	void AetEditor::DrawInspectorRegionData(AetRegion* aetRegion)
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

	void AetEditor::DrawInspectorLayerData(AetLayer* aetLayer)
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

	void AetEditor::DrawInspectorAnimationData(AnimationData* animationData)
	{
		if (ImGui::WideTreeNode("Animation Data"))
		{
			ImGui::Checkbox("Use Texture Mask", &animationData->UseTextureMask);

			if (ImGui::WideTreeNode("Properties"))
			{
				if (animationData->Properties != nullptr)
					DrawKeyFrameProperties(animationData->Properties.get());
				else ImGui::Text("nullptr");
				ImGui::TreePop();
			}

			if (ImGui::WideTreeNode("Perspective Properties"))
			{
				if (animationData->PerspectiveProperties != nullptr)
					DrawKeyFrameProperties(animationData->PerspectiveProperties.get());
				else ImGui::Text("nullptr");
				ImGui::TreePop();
			}

			ImGui::TreePop();
		}
	}

	void AetEditor::DrawKeyFrameProperties(KeyFrameProperties* properties)
	{
		size_t keyFrameIndex = 0;
		for (auto keyFrames = &properties->OriginX; keyFrames <= &properties->Opacity; keyFrames++)
			DrawKeyFrames(KeyFrameProperties::PropertyNames[keyFrameIndex++], keyFrames);
	}

	void AetEditor::DrawKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames)
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

	void AetEditor::DrawInspectorAetLayer(AetLayer* aetLayer)
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

	void AetEditor::DrawInspectorAetLyo(AetLyo* aetLyo)
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

	void AetEditor::DrawInspectorAetRegion(AetRegion* aetRegion)
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

	void AetEditor::DrawSetLoader()
	{
		if (fileViewer.DrawGui())
		{
			std::string aetPath = fileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(aetPath), "aet_") && EndsWithInsensitive(aetPath, ".bin"))
				OpenAetSet(aetPath.c_str());
		}
	}

	void AetEditor::DrawInspector()
	{
		ImGui::Text("Selection:");
		ImGui::Separator();

		if (selected.ItemPtr == nullptr)
		{
			ImGui::Text("nullptr");
			return;
		}

		switch (selected.Type)
		{
		case AetSelectionType::AetObj:
			DrawInspectorAetObj(selected.AetObj);
			break;
		case AetSelectionType::AetLayer:
			DrawInspectorAetLayer(selected.AetLayer);
			break;
		case AetSelectionType::AetLyo:
			DrawInspectorAetLyo(selected.AetLyo);
			break;
		case AetSelectionType::AetRegion:
			DrawInspectorAetRegion(selected.AetRegion);
			break;
		default:
			break;
		}
	}

	void AetEditor::DrawProperties()
	{
		if (selected.Type != AetSelectionType::AetObj || selected.AetObj == nullptr)
			return;

		float frame = timeline->GetFrame().Frames();
		AetMgr::Interpolate(selected.AetObj->AnimationData, frame, &currentProperties);

		ImGui::InputFloat2("Origin", glm::value_ptr(currentProperties.Origin));
		ImGui::InputFloat2("Position", glm::value_ptr(currentProperties.Position));
		ImGui::InputFloat("Rotation", &currentProperties.Rotation);
		ImGui::InputFloat("Scale", glm::value_ptr(currentProperties.Scale));
		ImGui::InputFloat("Opacity", &currentProperties.Opcaity);
	}

	bool AetEditor::OpenAetSet(const std::string& filePath)
	{
		if (!FileExists(filePath))
			return false;

		aetSet.release();
		aetSet = std::make_unique<AetSet>();
		aetSet->Name = GetFileName(filePath, false);
		aetSet->Load(filePath);

		return true;
	}
}
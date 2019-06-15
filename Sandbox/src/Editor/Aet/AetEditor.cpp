#include "AetEditor.h"
#include "../../FileSystem/MemoryStream.h"
#include "../../FileSystem/BinaryReader.h"
#include "../../FileSystem/FileHelper.h"

namespace Editor
{
	AetEditor::AetEditor(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
	}

	AetEditor::~AetEditor()
	{
	}

	void AetEditor::Initialize()
	{
		OpenAetSet(testAetPath);

		aetTimeline = std::make_unique<AetTimeline>();
		aetTimeline->InitializeTimelineGuiState();
	}

	void AetEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoBackground;

		if (ImGui::Begin("AetSet Loader##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Tree View##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetTreeViewChild##AetEditor");
			DrawTreeView();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("Aet Render Window##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetRenderWindowChild##AetEditor");
			DrawRenderWindow();
			ImGui::EndChild();
		}
		ImGui::End();

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
			aetTimeline->SetAetObj(selected.Type == SelectionType::AetObj ? selected.AetObj : nullptr);
			aetTimeline->DrawTimelineGui();
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

	void AetEditor::DrawAetObj(AetObj* aetObj)
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
			if (ImGui::Combo("Obj Type", &objTypeIndex, aetObjTypeNames.data(), aetObjTypeNames.size()))
				aetObj->Type = (AetObjType)objTypeIndex;

			ImGui::InputFloat("Loop Start", &aetObj->LoopStart);
			ImGui::InputFloat("Loop End", &aetObj->LoopEnd);
			ImGui::InputFloat("Start Frame", &aetObj->StartFrame);

			ImGui::TreePop();
		}

		if ((aetObj->Type == AetObjType_Pic))
			DrawSpriteData(aetObj->ReferencedSprite);

		if ((aetObj->Type == AetObjType_Eff))
			DrawLayerData(aetObj->ReferencedLayer);

		if ((aetObj->Type == AetObjType_Pic || aetObj->Type == AetObjType_Eff))
			DrawAnimationData(&aetObj->AnimationData);
	}

	void AetEditor::DrawSpriteData(SpriteEntry* spriteEntry)
	{
		if (ImGui::WideTreeNode("Sprite Data"))
		{
			if (spriteEntry != nullptr)
			{
				ImGui::InputScalarN("Dimensions", ImGuiDataType_S16, &spriteEntry->Width, 2);
				for (auto& sprite : spriteEntry->Sprites)
					ImGui::Text("%s", sprite.Name.c_str());
			}
			ImGui::TreePop();
		}
	}

	void AetEditor::DrawLayerData(AetLayer* aetLayer)
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

	void AetEditor::DrawAnimationData(AnimationData* animationData)
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

			if (ImGui::WideTreeNode("Properties Extra Data"))
			{
				if (animationData->PropertiesExtraData != nullptr)
					DrawKeyFrameProperties(animationData->PropertiesExtraData.get());
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

	void AetEditor::DrawAetLayer(AetLayer* aetLayer)
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

	void AetEditor::DrawAetLyo(AetLyo* aetLyo)
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

	void AetEditor::DrawSetLoader()
	{
		ImGui::PushItemWidth(ImGui::GetWindowWidth());
		ImGui::Text("AetSet Path:");

		bool openAetSet = false;
		{
			openAetSet |= ImGui::InputText("##AetSetPathInputText", aetSetPathBuffer, sizeof(aetSetPathBuffer), ImGuiInputTextFlags_EnterReturnsTrue);
			openAetSet |= ImGui::Button("Open AetSet...", ImVec2(ImGui::GetWindowWidth(), 0));
		}
		if (openAetSet)
			OpenAetSet(aetSetPathBuffer);

		ImGui::PopItemWidth();

		static std::vector<std::string> aetFilePaths;

		if (ImGui::Button("Refresh AetSet Paths...", ImVec2(ImGui::GetWindowWidth(), 0)))
		{
			aetFilePaths.clear();
			if (DirectoryExists(aetSetPathBuffer))
				aetFilePaths = GetFiles(aetSetPathBuffer);
		}

		if (ImGui::WideTreeNode("AetSet Directory"))
		{
			for (const auto& path : aetFilePaths)
			{
				ImGui::WideTreeNodeEx(path.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen);

				if (ImGui::IsItemClicked())
					OpenAetSet(path);
			}

			ImGui::TreePop();
		}
	}

	void AetEditor::DrawTreeView()
	{
		lastHovered = hovered;
		hovered = { SelectionType::None, nullptr };

		constexpr ImGuiTreeNodeFlags selectableTreeNodeFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		constexpr ImGuiTreeNodeFlags leafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		constexpr ImGuiTreeNodeFlags headerTreeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | selectableTreeNodeFlags;

		if (ImGui::WideTreeNodeEx((void*)aetSet.get(), headerTreeNodeFlags, "AetSet: %s", aetSet->Name.c_str()))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			for (auto& aetLyo : aetSet->AetLyos)
			{
				ImGuiTreeNodeFlags lyoNodeFlags = headerTreeNodeFlags;
				if (&aetLyo == selected.AetLyo || &aetLyo == lastHovered.AetLyo)
					lyoNodeFlags |= ImGuiTreeNodeFlags_Selected;

				bool aetLyoNodeOpen = ImGui::WideTreeNodeEx((void*)&aetLyo, lyoNodeFlags, "%s", aetLyo.Name.c_str());

				if (ImGui::IsItemClicked())
					selected = { SelectionType::AetLyo, &aetLyo };

				if (aetLyoNodeOpen)
				{
					int32_t layerIndex = 0;
					for (auto& aetLayer : aetLyo.AetLayers)
					{
						ImGui::PushID((void*)&aetLayer);

						ImGuiTreeNodeFlags layerNodeFlags = selectableTreeNodeFlags;
						if (&aetLayer == selected.AetLayer)
							layerNodeFlags |= ImGuiTreeNodeFlags_Selected;

						if (&aetLayer == lastHovered.AetLayer)
							ImGui::PushStyleColor(ImGuiCol_Text, GetColor(EditorColor_TextHighlight));

						AetLayer* rootLayer = &aetLyo.AetLayers.back();
						bool aetLayerNodeOpen = ImGui::WideTreeNodeEx("##AetLayerTreeNode", layerNodeFlags, (&aetLayer == rootLayer) ? "Root" : "Layer %d (%s)", aetLayer.Index, aetLayer.CommaSeparatedNames.c_str());

						if (&aetLayer == lastHovered.AetLayer)
							ImGui::PopStyleColor();

						aetLayer.Index = layerIndex++;

						if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
							selected = { SelectionType::AetLayer, &aetLayer };

						ImGui::OpenPopupOnItemClick(aetLayerContextMenuID, 1);

						bool openAddAetObjPopup = false;
						if (ImGui::BeginPopupContextItem(aetLayerContextMenuID))
						{
							openAddAetObjPopup = ImGui::MenuItem("Add new AetObj...");
							if (ImGui::MenuItem("Move Up")) {}
							if (ImGui::MenuItem("Move Down")) {}
							if (ImGui::MenuItem("Delete...")) {}
							ImGui::EndPopup();
						}

						if (openAddAetObjPopup)
							ImGui::OpenPopup(addAetObjPopupID);
						if (ImGui::BeginPopupModal(addAetObjPopupID, NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							if (ImGui::Combo("Obj Type", &newObjTypeIndex, aetObjTypeNames.data(), aetObjTypeNames.size()))
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
							if (ImGui::Button("Cancel", ImVec2(124, 0))) { ImGui::CloseCurrentPopup(); }

							ImGui::EndPopup();
						}

						if (aetLayerNodeOpen)
						{
							for (auto& aetObj : aetLayer.Objects)
							{
								ImGuiTreeNodeFlags objNodeFlags = leafTreeNodeFlags;
								if (&aetObj == selected.AetObj || &aetObj == hovered.AetObj)
									objNodeFlags |= ImGuiTreeNodeFlags_Selected;

								ImGui::WideTreeNodeEx((void*)&aetObj, objNodeFlags, "%s", aetObj.Name.c_str());

								if (ImGui::IsItemClicked())
									selected = { SelectionType::AetObj, &aetObj };

								if (aetObj.Type == AetObjType_Eff && (ImGui::IsItemHovered() || &aetObj == selected.AetObj))
									hovered = { SelectionType::AetLayer, aetObj.ReferencedLayer };
							}

							ImGui::TreePop();
						}

						ImGui::PopID();
					}

					ImGui::TreePop();
				}
			}
			ImGui::PopStyleVar();

			ImGui::TreePop();
		}
		else
		{
			selected = { SelectionType::None, nullptr };
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
		case SelectionType::AetObj:
			DrawAetObj(selected.AetObj);
			break;
		case SelectionType::AetLayer:
			DrawAetLayer(selected.AetLayer);
			break;
		case SelectionType::AetLyo:
			DrawAetLyo(selected.AetLyo);
			break;
		default:
			break;
		}
	}

	void AetEditor::DrawProperties()
	{
		struct AnimationProperties
		{
			float Origin[2];
			float Position[2];
			float Rotation;
			float Scale[2];
			float Opcaity;
		} properties = {};

		ImGui::InputFloat2("Origin", properties.Origin);
		ImGui::InputFloat2("Position", properties.Position);
		ImGui::InputFloat("Rotation", &properties.Rotation);
		ImGui::InputFloat("Scale", properties.Scale);
		ImGui::InputFloat("Opacity", &properties.Opcaity);
	}

	void AetEditor::DrawRenderWindow()
	{
		ImGui::Text("AetObj: Test");
		ImGui::Separator();
	}

	bool AetEditor::OpenAetSet(const std::string& filePath)
	{
		if (!FileExists(filePath))
			return false;

		MemoryStream stream(filePath);
		BinaryReader reader(&stream);
		{
			aetSet.release();
			aetSet = std::make_unique<AetSet>();
			aetSet->Name = GetFileName(filePath, false);
			aetSet->Read(reader);
		}
		reader.Close();

		return true;
	}
}
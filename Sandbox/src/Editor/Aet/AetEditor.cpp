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
	}

	void AetEditor::DrawGui()
	{
		if (false)
			ImGui::ShowDemoWindow();

		ImGui::BeginGroup();
		{
			static char aetSetPathBuffer[_MAX_PATH] = "dev_ram/aetset/";
			ImGui::InputText("AetSet Path", aetSetPathBuffer, sizeof(aetSetPathBuffer));
			if (ImGui::Button("Open AetSet...", { ImGui::CalcItemWidth(), 0 }))
				OpenAetSet(aetSetPathBuffer);
		}
		ImGui::EndGroup();

		ImGui::BeginChild("AetTreeViewChild##Aet");
		DrawTreeView();
		ImGui::EndChild();

		if (ImGui::Begin("Aet Inspector##Aet", nullptr, GetWindowFlags()))
		{
			ImGui::BeginChild("AetInspectorChild");
			DrawInspector();
			ImGui::EndChild();
		}
		ImGui::End();
	}

	const char* AetEditor::GetGuiName()
	{
		return u8"Aet Editor";
	}

	ImGuiWindowFlags AetEditor::GetWindowFlags()
	{
		return ImGuiWindowFlags_NoBackground;
	}

	void AetEditor::DrawAetObj(AetObj * aetObj)
	{
		ImGui::Text("AetObj: %s", aetObj->Name.c_str());

		if (ImGui::TreeNodeEx("Object Data", ImGuiTreeNodeFlags_DefaultOpen))
		{
			static char aetObjNameBuffer[255];
			strcpy_s(aetObjNameBuffer, aetObj->Name.c_str());

			if (ImGui::InputText("Name##AetObj", aetObjNameBuffer, sizeof(aetObjNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				aetObj->Name = std::string(aetObjNameBuffer);

			int objTypeIndex = aetObj->Type;
			if (ImGui::Combo("Obj Type", &objTypeIndex, aetObjTypeNames, IM_ARRAYSIZE(aetObjTypeNames)))
				aetObj->Type = (AetObjType)objTypeIndex;

			ImGui::InputFloat("Start Frame", &aetObj->StartFrame);
			ImGui::InputFloat("Loop Start", &aetObj->LoopStart);
			ImGui::InputFloat("Loop End", &aetObj->LoopEnd);

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
		if (ImGui::TreeNode("Sprite Data"))
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
		if (ImGui::TreeNode("Layer Data"))
		{
			if (aetLayer != nullptr)
			{
				ImGui::Text("AetLayer: 0x%p", aetLayer);
			}
			ImGui::TreePop();
		}
	}

	void AetEditor::DrawAnimationData(AnimationData* animationData)
	{
		if (ImGui::TreeNode("Animation Data"))
		{
			ImGui::Checkbox("Use Texture Mask", &animationData->UseTextureMask);

			if (ImGui::TreeNode("Properties"))
			{
				if (animationData->Properties != nullptr)
					DrawKeyFrameProperties(animationData->Properties.get());
				else ImGui::Text("nullptr");
				ImGui::TreePop();
			}

			if (ImGui::TreeNode("Properties Extra Data"))
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
		static const char* propertyNames[] = { "Origin X", "Origin Y", "Position X", "Position Y", "Rotation", "Scale X", "Scale Y", "Opcatiy" };

		size_t keyFrameIndex = 0;
		for (auto keyFrames = &properties->OriginX; keyFrames <= &properties->Opacity; keyFrames++)
			DrawKeyFrames(propertyNames[keyFrameIndex++], keyFrames);
	}

	void AetEditor::DrawKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames)
	{
		if (ImGui::TreeNode(name))
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

		for (auto& aetObj : aetLayer->Objects)
			ImGui::BulletText(aetObj.Name.c_str());
	}

	void AetEditor::DrawAetLyo(AetLyo* aetLyo)
	{
		ImGui::Text("AetLyo: %s", aetLyo->Name.c_str());

		if (ImGui::TreeNodeEx("Lyo Data", ImGuiTreeNodeFlags_DefaultOpen))
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

	void AetEditor::DrawTreeView()
	{
		if (ImGui::TreeNodeEx((void*)aetSet.get(), ImGuiTreeNodeFlags_DefaultOpen, "AetSet: %s", aetSet->Name.c_str()))
		{
			ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, ImGui::GetFontSize() * 1.5f);

			for (auto& aetLyo : aetSet->AetLyos)
			{
				ImGuiTreeNodeFlags lyoNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
				if (&aetLyo == selected.AetLyo)
					lyoNodeFlags |= ImGuiTreeNodeFlags_Selected;

				bool aetLyoNodeOpen = ImGui::TreeNodeEx((void*)&aetLyo, lyoNodeFlags, "%s", aetLyo.Name.c_str());

				if (ImGui::IsItemClicked())
				{
					selected.AetLyo = &aetLyo;
					selected.Type = SelectionType::AetLyo;
				}

				if (aetLyoNodeOpen)
				{
					for (auto& aetLayer : aetLyo.AetLayers)
					{
						ImGui::PushID((void*)&aetLayer);

						ImGuiTreeNodeFlags layerNodeFlags = ImGuiTreeNodeFlags_None; // ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
						if (&aetLayer == selected.AetLayer)
							layerNodeFlags |= ImGuiTreeNodeFlags_Selected;

						AetLayer* rootLayer = &aetLyo.AetLayers.back();
						bool aetLayerNodeOpen = ImGui::TreeNodeEx((void*)&aetLayer, layerNodeFlags, (&aetLayer == rootLayer) ? "Root" : "Layer");

						if (ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
						{
							selected.AetLayer = &aetLayer;
							selected.Type = SelectionType::AetLayer;
						}

						ImGui::OpenPopupOnItemClick(aetLayerContextMenuID, 1);

						bool openAddAetObjPopup = false;
						if (ImGui::BeginPopupContextItem(aetLayerContextMenuID))
						{
							openAddAetObjPopup = ImGui::MenuItem("Add new AetObj...");
							ImGui::EndPopup();
						}

						if (openAddAetObjPopup)
							ImGui::OpenPopup(addAetObjPopupID);
						if (ImGui::BeginPopupModal(addAetObjPopupID, NULL, ImGuiWindowFlags_AlwaysAutoResize))
						{
							if (ImGui::Combo("Obj Type", &newObjTypeIndex, aetObjTypeNames, IM_ARRAYSIZE(aetObjTypeNames)))
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
								ImGuiTreeNodeFlags objNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
								if (&aetObj == selected.AetObj)
									objNodeFlags |= ImGuiTreeNodeFlags_Selected;

								ImGui::TreeNodeEx((void*)&aetObj, objNodeFlags, "%s", aetObj.Name.c_str());

								if (ImGui::IsItemClicked())
								{
									selected.AetObj = &aetObj;
									selected.Type = SelectionType::AetObj;
								}
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
			selected.Type = SelectionType::None;
			selected.ItemPtr = nullptr;
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

	void AetEditor::OpenAetSet(const char* filePath)
	{
		if (!FileExists(filePath))
			return;

		MemoryStream stream(filePath);
		BinaryReader reader(&stream);
		{
			aetSet.release();
			aetSet = std::make_unique<AetSet>();
			aetSet->Name = GetFileName(filePath, false);
			aetSet->Read(reader);
		}
		reader.Close();
	}
}
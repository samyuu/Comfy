#include "AetEditor.h"
#include "FileSystem/MemoryStream.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	static AetEditor* AetEditorInstance;

	static bool AetEditorSpriteGetter(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite)
	{
		SprSet* sprSet = AetEditorInstance->GetSprSet();

		if (inSprite == nullptr || sprSet == nullptr)
			return false;

		if (inSprite->SpriteCache != nullptr)
		{
		from_sprite_cache:
			*outTexture = sprSet->TxpSet->Textures[inSprite->SpriteCache->TextureIndex].get();
			*outSprite = inSprite->SpriteCache;
			return true;
		}

		for (auto& sprite : sprSet->Sprites)
		{
			if (EndsWith(inSprite->Name, sprite.Name))
			{
				inSprite->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	AetEditor::AetEditor(Application* parent, PvEditor* editor) : IEditorComponent(parent, editor)
	{
		AetEditorInstance = this;

		treeView = std::make_unique<AetTreeView>();
		inspector = std::make_unique<AetInspector>();
		timeline = std::make_unique<AetTimeline>();
		renderWindow = std::make_unique<AetRenderWindow>(&AetEditorSpriteGetter);
	}

	AetEditor::~AetEditor()
	{
	}

	void AetEditor::Initialize()
	{
		treeView->Initialize();
		inspector->Initialize();
		timeline->InitializeTimelineGuiState();
		renderWindow->Initialize();

		LoadAetSet(testAetPath);
	}

	void AetEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoBackground;

		if (ImGui::Begin("AetSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawAetSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin("SprSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("SprSetLoaderChild##AetEditor");
			DrawSprSetLoader();
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
			renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			renderWindow->SetCurrentFrame(timeline->GetFrame().Frames());
			renderWindow->DrawGui();
		}
		ImGui::End();
		RenderWindowBase::PopWindowPadding();

		if (ImGui::Begin("Aet Inspector##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetInspectorChild##AetEditor");
			inspector->DrawGui(aetSet.get(), treeView->GetSelected());
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
			AetItemTypePtr selected = treeView->GetSelected();
			timeline->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
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

	void AetEditor::DrawAetSetLoader()
	{
		if (aetFileViewer.DrawGui())
		{
			std::string aetPath = aetFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(aetPath), "aet_") && EndsWithInsensitive(aetPath, ".bin"))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			std::string sprPath = sprFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
				LoadSprSet(sprPath);
		}
	}

	void AetEditor::DrawProperties()
	{
		AetItemTypePtr selected = treeView->GetSelected();

		if (selected.Type() == AetSelectionType::AetObj && selected.AetObj != nullptr && selected.AetObj->AnimationData.Properties != nullptr)
		{
			float frame = timeline->GetFrame().Frames();
			AetMgr::Interpolate(selected.AetObj->AnimationData, &currentProperties, frame);
		}
		else
		{
			currentProperties = {};
		}

		ImGui::InputFloat2("Origin", glm::value_ptr(currentProperties.Origin));
		ImGui::InputFloat2("Position", glm::value_ptr(currentProperties.Position));
		ImGui::InputFloat("Rotation", &currentProperties.Rotation);
		ImGui::InputFloat("Scale", glm::value_ptr(currentProperties.Scale));
		ImGui::InputFloat("Opacity", &currentProperties.Opacity);
	}

	bool AetEditor::LoadAetSet(const std::string& filePath)
	{
		if (!FileExists(filePath))
			return false;

		aetSet.reset();
		aetSet = std::make_unique<AetSet>();
		aetSet->Name = GetFileName(filePath, false);
		aetSet->Load(filePath);

		OnAetSetLoaded();
		return true;
	}

	bool AetEditor::LoadSprSet(const std::string& filePath)
	{
		if (!FileExists(filePath))
			return false;

		std::vector<uint8_t> fileBuffer;
		ReadAllBytes(filePath, &fileBuffer);

		sprSet.reset();
		sprSet = std::make_unique<SprSet>();
		sprSet->Parse(fileBuffer.data());
		sprSet->Name = GetFileName(filePath, false);

		for (int i = 0; i < sprSet->TxpSet->Textures.size(); i++)
		{
			sprSet->TxpSet->Textures[i]->Texture2D = std::make_shared<Texture2D>();
			sprSet->TxpSet->Textures[i]->Texture2D->Upload(sprSet->TxpSet->Textures[i].get());
		}

		OnSprSetLoaded();
		return true;
	}

	void AetEditor::OnAetSetLoaded()
	{
		treeView->ResetSelectedItem();
	}

	void AetEditor::OnSprSetLoaded()
	{
		if (aetSet != nullptr)
			aetSet->ClearSpriteCache();
	}
}
#include "AetEditor.h"
#include "AetIcons.h"
#include "FileSystem/BinaryReader.h"
#include "FileSystem/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Editor
{
	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		spriteGetterFunction = [this](const AetSprite* inSprite, const Texture** outTexture, const Sprite** outSprite) { return false; };

		treeView = MakeUnique<AetTreeView>();
		layerView = MakeUnique<AetLayerView>();
		inspector = MakeUnique<AetInspector>();
		timeline = MakeUnique<AetTimeline>();
		renderWindow = MakeUnique<AetRenderWindow>(&spriteGetterFunction);
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

		if (testAetPath != nullptr)
			LoadAetSet(testAetPath);
		if (testSprPath != nullptr)
			LoadSprSet(testSprPath);
	}

	void AetEditor::DrawGui()
	{
		ImGui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoBackground;

		if (ImGui::Begin(ICON_SETLOADER "  AetSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawAetSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_SETLOADER "  SprSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			ImGui::BeginChild("SprSetLoaderChild##AetEditor");
			DrawSprSetLoader();
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_TREEVIEW "  Aet Tree View##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetTreeViewChild##AetEditor", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			treeView->DrawGui(aetSet.get());
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_AETLAYERS "  Aet Layers##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetLayerViewChild##AetEditor", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			layerView->DrawGui();
			ImGui::EndChild();
		}
		ImGui::End();

		RenderWindowBase::PushWindowPadding();
		if (ImGui::Begin(ICON_RENDERWINDOW "  Aet Window##AetEditor", nullptr, windowFlags))
		{
			renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			renderWindow->SetIsPlayback(timeline->GetIsPlayback());
			renderWindow->SetCurrentFrame(timeline->GetCursorFrame().Frames());
			renderWindow->DrawGui();
		}
		ImGui::End();
		RenderWindowBase::PopWindowPadding();

		if (ImGui::Begin(ICON_INSPECTOR "  Aet Inspector##AetEditor", nullptr, windowFlags))
		{
			ImGui::BeginChild("AetInspectorChild##AetEditor");
			inspector->DrawGui(treeView->GetActiveAet(), treeView->GetSelected());
			ImGui::EndChild();
		}
		ImGui::End();

		if (ImGui::Begin(ICON_FA_CLOCK "  Aet Timeline##AetEditor", nullptr))
		{
			AetItemTypePtr selected = treeView->GetSelected();
			timeline->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			timeline->DrawTimelineGui();
		}
		ImGui::End();

		UpdateFileLoading();
	}

	const char* AetEditor::GetGuiName() const
	{
		return u8"Aet Editor";
	}

	ImGuiWindowFlags AetEditor::GetWindowFlags() const
	{
		return BaseWindow::GetNoWindowFlags();
	}

	void AetEditor::UpdateFileLoading()
	{
		if (sprSetFileLoader != nullptr && sprSetFileLoader->GetIsLoaded())
		{
			sprSet = MakeUnique<SprSet>();
			sprSetFileLoader->Parse(sprSet.get());
			sprSet->Name = GetFileName(sprSetFileLoader->GetFilePath(), false);
			sprSet->TxpSet->UploadAll();

			OnSprSetLoaded();
			sprSetFileLoader.reset();
		}
	}

	void AetEditor::DrawAetSetLoader()
	{
		if (aetFileViewer.DrawGui())
		{
			const std::string& aetPath = aetFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(aetPath), "aet_") && (EndsWithInsensitive(aetPath, ".bin") || EndsWithInsensitive(aetPath, ".aec")))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			const std::string& sprPath = sprFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
				LoadSprSet(sprPath);
		}
	}

	bool AetEditor::LoadAetSet(const std::string& filePath)
	{
		const std::wstring widePath = Utf8ToUtf16(filePath);
		if (!FileExists(widePath))
			return false;

		aetSet = MakeUnique<AetSet>();
		aetSet->Name = GetFileName(filePath, false);
		aetSet->Load(widePath);

		OnAetSetLoaded();
		return true;
	}

	bool AetEditor::LoadSprSet(const std::string& filePath)
	{
		if (!FileExists(Utf8ToUtf16(filePath)))
			return false;

		if (sprSetFileLoader != nullptr)
			return false;

		sprSetFileLoader = MakeUnique<FileLoader>(filePath);
		sprSetFileLoader->LoadAsync();

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
	
		spriteGetterFunction = [this](const AetSprite* inSprite, const Texture** outTexture, const Sprite** outSprite) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), inSprite, outTexture, outSprite); };
	}
}
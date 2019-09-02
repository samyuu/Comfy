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
		commandManager = MakeUnique<AetCommandManager>();

		spriteGetterFunction = [this](const AetSprite* inSprite, const Texture** outTexture, const Sprite** outSprite) { return false; };

		treeView = MakeUnique<AetTreeView>();
		layerView = MakeUnique<AetLayerView>();
		inspector = MakeUnique<AetInspector>(commandManager.get());
		timeline = MakeUnique<AetTimeline>();
		renderWindow = MakeUnique<AetRenderWindow>(&spriteGetterFunction);
		historyWindow = MakeUnique<AetHistoryWindow>(commandManager.get());
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

		if (debugAetPath != nullptr)
			LoadAetSet(debugAetPath);
		if (debugSprPath != nullptr)
			LoadSprSet(debugSprPath);
	}

	void AetEditor::DrawGui()
	{
		Gui::GetCurrentWindow()->Hidden = true;
		constexpr ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None; // ImGuiWindowFlags_NoBackground;

		if (Gui::Begin(ICON_SETLOADER "  AetSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			Gui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawAetSetLoader();
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_SETLOADER "  SprSet Loader##AetEditor", nullptr, ImGuiWindowFlags_None))
		{
			Gui::BeginChild("SprSetLoaderChild##AetEditor");
			DrawSprSetLoader();
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_TREEVIEW "  Aet Tree View##AetEditor", nullptr, windowFlags))
		{
			Gui::BeginChild("AetTreeViewChild##AetEditor", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);
			treeView->DrawGui(editorAetSet);
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_AETLAYERS "  Aet Layers##AetEditor", nullptr, windowFlags))
		{
			Gui::BeginChild("AetLayerViewChild##AetEditor", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			layerView->DrawGui();
			Gui::EndChild();
		}
		Gui::End();

		RenderWindowBase::PushWindowPadding();
		if (Gui::Begin(ICON_RENDERWINDOW "  Aet Window##AetEditor", nullptr, windowFlags))
		{
			renderWindow->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			renderWindow->SetIsPlayback(timeline->GetIsPlayback());
			renderWindow->SetCurrentFrame(timeline->GetCursorFrame().Frames());
			renderWindow->DrawGui();
		}
		Gui::End();
		RenderWindowBase::PopWindowPadding();

		if (Gui::Begin(ICON_INSPECTOR "  Aet Inspector##AetEditor", nullptr, windowFlags))
		{
			Gui::BeginChild("AetInspectorChild##AetEditor");
			inspector->DrawGui(treeView->GetActiveAet(), treeView->GetSelected());
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_TIMELINE "  Aet Timeline##AetEditor", nullptr))
		{
			AetItemTypePtr selected = treeView->GetSelected();
			timeline->SetActive(treeView->GetActiveAet(), treeView->GetSelected());
			timeline->DrawTimelineGui();
		}
		Gui::End();

		if (Gui::Begin(ICON_HISTORY "  Aet Editor History##AetEditor", nullptr))
		{
			historyWindow->DrawGui();
		}
		Gui::End();

		UpdateFileLoading();
		commandManager->ExecuteClearCommandQueue();
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
			const String& aetPath = aetFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(aetPath), "aet_") && (EndsWithInsensitive(aetPath, ".bin") || EndsWithInsensitive(aetPath, ".aec")))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			const String& sprPath = sprFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
				LoadSprSet(sprPath);
		}
	}

	bool AetEditor::LoadAetSet(const String& filePath)
	{
		const WideString widePath = Utf8ToUtf16(filePath);
		if (!FileExists(widePath))
			return false;

		editorAetSet = MakeRef<AetSet>();
		editorAetSet->Name = GetFileName(filePath, false);
		editorAetSet->Load(widePath);

		OnAetSetLoaded();
		return true;
	}

	bool AetEditor::LoadSprSet(const String& filePath)
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
		if (editorAetSet != nullptr)
			editorAetSet->ClearSpriteCache();
	
		spriteGetterFunction = [this](const AetSprite* inSprite, const Texture** outTexture, const Sprite** outSprite) { return AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), inSprite, outTexture, outSprite); };
	}
}
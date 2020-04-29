#include "AetEditor.h"
#include "AetIcons.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/FileHelper.h"
#include "Misc/StringHelper.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		commandManager = MakeUnique<AetCommandManager>();

		spriteGetterFunction = [](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr) { return false; };

		treeView = MakeUnique<AetTreeView>(commandManager.get(), &selectedAetItem, &cameraSelectedAetItem);
		inspector = MakeUnique<AetInspector>(commandManager.get(), &spriteGetterFunction, &previewData);
		// contentView = MakeUnique<AetContentView>();
		timeline = MakeUnique<AetTimeline>();
		renderWindow = MakeUnique<AetRenderWindow>(commandManager.get(), &spriteGetterFunction, &selectedAetItem, &cameraSelectedAetItem, &previewData);
		historyWindow = MakeUnique<AetHistoryWindow>(commandManager.get());
	}

	AetEditor::~AetEditor()
	{
	}

	void AetEditor::Initialize()
	{
		treeView->Initialize();
		inspector->Initialize();
		// contentView->Initialize();
		timeline->Initialize();
		renderWindow->Initialize();

		// DEBUG: Auto load specified files
		if (debugAetPath != nullptr && IO::FileExists(debugAetPath))
			LoadAetSet(debugAetPath);
		if (debugSprPath != nullptr && IO::FileExists(debugSprPath))
			LoadSprSet(debugSprPath);
	}

	void AetEditor::DrawGui()
	{
		Gui::GetCurrentWindow()->Hidden = true;

		if (Gui::Begin(ICON_SETLOADER "  AetSet Loader##AetEditor"))
		{
			Gui::BeginChild("AetSetLoaderChild##AetEditor");
			DrawAetSetLoader();
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_SETLOADER "  SprSet Loader##AetEditor"))
		{
			Gui::BeginChild("SprSetLoaderChild##AetEditor");
			DrawSprSetLoader();
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_TREEVIEW "  Aet Tree View##AetEditor"))
		{
			Gui::BeginChild("AetTreeViewChild##AetEditor", vec2(0.0f, 0.0f), false, ImGuiWindowFlags_HorizontalScrollbar);
			treeView->DrawGui(editorAetSet);
			Gui::EndChild();
		}
		Gui::End();

		// HACK: The way the window padding works here is far from optimal
		RenderWindowBase::PushWindowPadding();
		if (Gui::Begin(ICON_RENDERWINDOW "  Aet Window##AetEditor"))
		{
			renderWindow->SetIsPlayback(timeline->GetIsPlayback());
			renderWindow->SetCurrentFrame(timeline->GetCursorFrame().Frames());
			renderWindow->DrawGui();
		}
		Gui::End();
		RenderWindowBase::PopWindowPadding();

		if (Gui::Begin(ICON_INSPECTOR "  Aet Inspector##AetEditor"))
		{
			// TODO: The selected item pointers should be passed in as pointers in their constructor
			Gui::BeginChild("AetInspectorChild##AetEditor");
			inspector->SetIsPlayback(timeline->GetIsPlayback());
			inspector->SetCurrentFrame(timeline->GetCursorFrame().Frames());
			inspector->DrawGui(selectedAetItem);
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_TIMELINE "  Aet Timeline##AetEditor"))
		{
			// TODO: The selected item pointers should be passed in as pointers in their constructor
			timeline->SetActive(selectedAetItem);
			timeline->DrawTimelineGui();
		}
		Gui::End();

		if (Gui::Begin(ICON_HISTORY "  Aet Editor History##AetEditor"))
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
			sprSet->Name = IO::GetFileName(sprSetFileLoader->GetFilePath(), false);
			sprSet->TexSet->UploadAll(sprSet.get());

			OnSprSetLoaded();
			sprSetFileLoader.reset();
		}
	}

	void AetEditor::DrawAetSetLoader()
	{
		if (aetFileViewer.DrawGui())
		{
			const std::string& aetPath = aetFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(IO::GetFileName(aetPath), "aet_") && (EndsWithInsensitive(aetPath, ".bin") || EndsWithInsensitive(aetPath, ".aec")))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			const std::string& sprPath = sprFileViewer.GetFileToOpen();
			if (StartsWithInsensitive(IO::GetFileName(sprPath), "spr_") && EndsWithInsensitive(sprPath, ".bin"))
				LoadSprSet(sprPath);
		}
	}

	bool AetEditor::LoadAetSet(const std::string& filePath)
	{
		if (!IO::FileExists(filePath))
			return false;

		editorAetSet = MakeRef<Aet::AetSet>();
		editorAetSet->Name = IO::GetFileName(filePath, false);
		editorAetSet->Load(filePath);

		OnAetSetLoaded();
		return true;
	}

	bool AetEditor::LoadSprSet(const std::string& filePath)
	{
		if (!IO::FileExists(filePath))
			return false;

		if (sprSetFileLoader != nullptr)
			return false;

		sprSetFileLoader = MakeUnique<IO::FileLoader>(filePath);
		if (asyncFileLoading)
		{
			sprSetFileLoader->LoadAsync();
		}
		else
		{
			sprSetFileLoader->LoadSync();
		}

		return true;
	}

	void AetEditor::OnAetSetLoaded()
	{
		commandManager->Clear();
		treeView->ResetSelectedItems();
	}

	void AetEditor::OnSprSetLoaded()
	{
		if (editorAetSet != nullptr)
			editorAetSet->ClearSpriteCache();

		spriteGetterFunction = [this](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr)
		{
			return Aet::AetRenderer::SpriteNameSprSetSpriteGetter(sprSet.get(), source, outTex, outSpr);
		};
	}
}

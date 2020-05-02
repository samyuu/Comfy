#include "AetEditor.h"
#include "AetIcons.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringHelper.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		commandManager = std::make_unique<AetCommandManager>();

		spriteGetterFunction = [](const Aet::VideoSource* source, const Tex** outTex, const Spr** outSpr) { return false; };

		treeView = std::make_unique<AetTreeView>(commandManager.get(), &selectedAetItem, &cameraSelectedAetItem);
		inspector = std::make_unique<AetInspector>(commandManager.get(), &spriteGetterFunction, &previewData);
		// contentView = std::make_unique<AetContentView>();
		timeline = std::make_unique<AetTimeline>();
		renderWindow = std::make_unique<AetRenderWindow>(commandManager.get(), &spriteGetterFunction, &selectedAetItem, &cameraSelectedAetItem, &previewData);
		historyWindow = std::make_unique<AetHistoryWindow>(commandManager.get());
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
		if (debugAetPath != nullptr && IO::File::Exists(debugAetPath))
			LoadAetSet(debugAetPath);
		if (debugSprPath != nullptr && IO::File::Exists(debugSprPath))
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
			sprSet = std::make_unique<SprSet>();
			sprSetFileLoader->Parse(*sprSet);
			sprSet->Name = IO::Path::GetFileName(sprSetFileLoader->GetFilePath(), false);
			sprSet->TexSet->UploadAll(sprSet.get());

			OnSprSetLoaded();
			sprSetFileLoader.reset();
		}
	}

	void AetEditor::DrawAetSetLoader()
	{
		if (aetFileViewer.DrawGui())
		{
			const auto aetPath = aetFileViewer.GetFileToOpen();
			const auto fileName = IO::Path::GetFileName(aetPath);
			if (StartsWithInsensitive(fileName, "aet_") && (EndsWithInsensitive(fileName, ".bin") || EndsWithInsensitive(fileName, ".aec")))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			const auto sprPath = sprFileViewer.GetFileToOpen();
			const auto fileName = IO::Path::GetFileName(sprPath);
			if (StartsWithInsensitive(fileName, "spr_") && EndsWithInsensitive(fileName, ".bin"))
				LoadSprSet(sprPath);
		}
	}

	bool AetEditor::LoadAetSet(std::string_view filePath)
	{
		auto loadedAetSet = IO::File::Load<Aet::AetSet>(filePath);
		if (loadedAetSet == nullptr)
			return false;

		editorAetSet = std::move(loadedAetSet);
		editorAetSet->Name = IO::Path::GetFileName(filePath, false);

		OnAetSetLoaded();
		return true;
	}

	bool AetEditor::LoadSprSet(std::string_view filePath)
	{
		if (!IO::File::Exists(filePath))
			return false;

		if (sprSetFileLoader != nullptr)
			return false;

		sprSetFileLoader = std::make_unique<IO::AsyncFileLoader>(filePath);
		if (asyncFileLoading)
			sprSetFileLoader->LoadAsync();
		else
			sprSetFileLoader->LoadSync();

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

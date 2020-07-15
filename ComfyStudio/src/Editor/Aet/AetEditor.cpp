#include "AetEditor.h"
#include "AetIcons.h"
#include "IO/Stream/Manipulator/StreamReader.h"
#include "IO/File.h"
#include "IO/Path.h"
#include "Misc/StringUtil.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	constexpr ImGuiTreeNodeFlags LeafTreeNodeFlags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet;

	AetEditor::AetEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
	}

	void AetEditor::OnFirstFrame()
	{
		commandManager = std::make_unique<AetCommandManager>();
		renderer = std::make_unique<Render::Renderer2D>();

		treeView = std::make_unique<AetTreeView>(*commandManager, selectedAetItem, cameraSelectedAetItem);
		inspector = std::make_unique<AetInspector>(*commandManager, *renderer, previewData);
		// contentView = std::make_unique<AetContentView>();
		timeline = std::make_unique<AetTimeline>();
		renderWindow = std::make_unique<AetRenderWindow>(*commandManager, *renderer, selectedAetItem, cameraSelectedAetItem, previewData);
		historyWindow = std::make_unique<AetHistoryWindow>(*commandManager);

		treeView->OnFirstFrame();
		inspector->Initialize();
		// contentView->OnFirstFrame();
		timeline->Initialize();

		// DEBUG: Auto load specified files
		if (!debugAetPath.empty() && IO::File::Exists(debugAetPath))
			LoadAetSet(debugAetPath);
		if (!debugSprPath.empty() && IO::File::Exists(debugSprPath))
			StartAsyncLoadSprSet(debugSprPath);
	}

	const char* AetEditor::GetName() const
	{
		return "Aet Editor";
	}

	ImGuiWindowFlags AetEditor::GetFlags() const
	{
		return BaseWindow::NoWindowFlags;
	}

	void AetEditor::Gui()
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
			treeView->Gui(editorAetSet);
			Gui::EndChild();
		}
		Gui::End();

		renderWindow->SetIsPlayback(timeline->GetIsPlayback());
		renderWindow->SetCurrentFrame(timeline->GetCursorFrame().Frames());
		renderWindow->BeginEndGui(ICON_RENDERWINDOW "  Aet Window##AetEditor");

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
			historyWindow->Gui();
		}
		Gui::End();

		UpdateCheckAsyncFileLoading();
		commandManager->ExecuteClearCommandQueue();
	}

	void AetEditor::UpdateCheckAsyncFileLoading()
	{
		if (sprSetLoadFuture.valid() && sprSetLoadFuture._Is_ready())
		{
			editorSprSet = sprSetLoadFuture.get();
			if (editorSprSet != nullptr)
			{
				editorSprSet->Name = IO::Path::GetFileName(sprSetFilePath, false);

				if (Util::MatchesInsensitive(IO::Path::GetExtension(sprSetFilePath), ".spr"))
				{
					const auto sprDBPath = IO::Path::ChangeExtension(sprSetFilePath, ".spi");
					const auto sprDB = IO::File::Load<Database::SprDB>(sprDBPath);
					
					if (sprDB != nullptr)
					{
						const auto matchingEntry = FindIfOrNull(sprDB->Entries, [&](const auto& e) { return Util::MatchesInsensitive(e.Name, editorSprSet->Name); });
						if (matchingEntry != nullptr)
							editorSprSet->ApplyDBNames(*matchingEntry);
					}
				}
			}

			OnSprSetLoaded();
		}
	}

	void AetEditor::DrawAetSetLoader()
	{
		if (aetFileViewer.DrawGui())
		{
			const auto aetPath = aetFileViewer.GetFileToOpen();
			const auto fileName = IO::Path::GetFileName(aetPath);

			if (Util::StartsWithInsensitive(fileName, "aet_") && (Util::EndsWithInsensitive(fileName, ".bin") || Util::EndsWithInsensitive(fileName, ".aec")))
				LoadAetSet(aetPath);
		}
	}

	void AetEditor::DrawSprSetLoader()
	{
		if (sprFileViewer.DrawGui())
		{
			const auto sprPath = sprFileViewer.GetFileToOpen();
			const auto fileName = IO::Path::GetFileName(sprPath);

			if (Util::StartsWithInsensitive(fileName, "spr_") && (Util::EndsWithInsensitive(fileName, ".bin") || Util::EndsWithInsensitive(fileName, ".spr")))
				StartAsyncLoadSprSet(sprPath);
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

	bool AetEditor::StartAsyncLoadSprSet(std::string_view filePath)
	{
		sprSetFilePath = std::string(filePath);
		sprSetLoadFuture = IO::File::LoadAsync<SprSet>(filePath);
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

		renderer->Aet().SetSprGetter([&](const Aet::VideoSource& source) -> Render::TexSpr
		{
			return Render::SprSetNameStringSprGetter(source, editorSprSet.get());
		});
	}
}

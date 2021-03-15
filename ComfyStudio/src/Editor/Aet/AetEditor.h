#pragma once
#include "AetSelection.h"
#include "Core/AetTreeView.h"
#include "Core/AetInspector.h"
#include "Core/AetContentView.h"
#include "Timeline/AetTimeline.h"
#include "RenderWindow/AetRenderWindow.h"
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Common/UndoHistoryWindow.h"
#include "Database/SprDB.h"
#include "Undo/Undo.h"
#include "IO/File.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Comfy::Studio::Editor
{
	class AetEditor : public IEditorComponent
	{
	public:
		AetEditor(ComfyStudioApplication& parent, EditorManager& editor);
		~AetEditor() = default;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	public:
		inline Graphics::AetSet* GetAetSet() { return editorAetSet.get(); }
		inline Graphics::SprSet* GetSprSet() { return editorSprSet.get(); }

	private:
		void UpdateCheckAsyncFileLoading();

		void DrawAetSetLoader();
		void DrawSprSetLoader();

		bool LoadAetSet(std::string_view filePath);
		bool StartAsyncLoadSprSet(std::string_view filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();

	private:
		Undo::UndoManager undoManager = {};

		std::unique_ptr<Render::Renderer2D> renderer = nullptr;

		struct
		{
			AetItemTypePtr selectedAetItem = {};
			AetItemTypePtr cameraSelectedAetItem = {};
			AetRenderPreviewData previewData;
		};

		std::unique_ptr<AetTreeView> treeView;
		std::unique_ptr<AetInspector> inspector;
		std::unique_ptr<AetContentView> contentView;
		std::unique_ptr<AetTimeline> timeline;
		std::unique_ptr<AetRenderWindow> renderWindow;
		UndoHistoryWindow historyWindow = { undoManager };

		struct
		{
			std::shared_ptr<Graphics::AetSet> editorAetSet = nullptr;
			std::unique_ptr<Graphics::SprSet> editorSprSet = nullptr;
		};

	private:
		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		std::string sprSetFilePath;
		std::future<std::unique_ptr<Graphics::SprSet>> sprSetLoadFuture;

		static constexpr std::string_view debugAetPath = "dev_ram/aetset/aet_gam/aet_gam_cmn.bin";
		static constexpr std::string_view debugSprPath = "dev_ram/sprset/spr_gam/spr_gam_cmn.bin";
	};
}

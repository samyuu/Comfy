#pragma once
#include "AetSelection.h"
#include "Core/AetTreeView.h"
#include "Core/AetInspector.h"
#include "Core/AetContentView.h"
#include "Timeline/AetTimeline.h"
#include "RenderWindow/AetRenderWindow.h"
#include "Command/AetHistoryWindow.h"
#include "Command/AetCommandManager.h"
#include "Editor/Core/IEditorComponent.h"
#include "Graphics/Auth2D/Aet/AetRenderer.h"
#include "IO/AsyncFileLoader.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Comfy::Studio::Editor
{
	class AetEditor : public IEditorComponent
	{
	public:
		AetEditor(Application* parent, EditorManager* editor);
		~AetEditor();

		void Initialize() override;
		void DrawGui() override;
		const char* GetGuiName() const override;
		ImGuiWindowFlags GetWindowFlags() const override;

		inline Graphics::Aet::AetSet* GetAetSet() { return editorAetSet.get(); }
		inline Graphics::SprSet* GetSprSet() { return sprSet.get(); }

	private:
		void UpdateFileLoading();

		void DrawAetSetLoader();
		void DrawSprSetLoader();

		bool LoadAetSet(std::string_view filePath);
		bool LoadSprSet(std::string_view filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();

	private:
		std::unique_ptr<AetCommandManager> commandManager = {};
		Graphics::Aet::SpriteGetterFunction spriteGetterFunction;
		
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
		std::unique_ptr<AetHistoryWindow> historyWindow;

		struct
		{
			std::shared_ptr<Graphics::Aet::AetSet> editorAetSet = nullptr;
			std::unique_ptr<Graphics::SprSet> sprSet = nullptr;
		};

	private:
		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		// DEBUG: Disabled for now to remove one possible case of failure
		const bool asyncFileLoading = false;
		std::unique_ptr<IO::AsyncFileLoader> sprSetFileLoader;

		static constexpr const char* debugAetPath = "dev_ram/aetset/aet_gam/aet_gam_cmn.bin";
		static constexpr const char* debugSprPath = "dev_ram/sprset/spr_gam/spr_gam_cmn.bin";
	};
}

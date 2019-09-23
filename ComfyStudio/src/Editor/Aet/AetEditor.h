#pragma once
#include "AetSelection.h"
#include "Core/AetTreeView.h"
#include "Core/AetLayerView.h"
#include "Core/AetInspector.h"
#include "Timeline/AetTimeline.h"
#include "RenderWindow/AetRenderWindow.h"
#include "Command/AetHistoryWindow.h"
#include "Command/AetCommandManager.h"
#include "Editor/Core/IEditorComponent.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/FileLoader.h"
#include "ImGui/Widgets/FileViewer.h"

namespace Editor
{
	using namespace FileSystem;

	class AetEditor : public IEditorComponent
	{
	public:
		AetEditor(Application* parent, EditorManager* editor);
		~AetEditor();

		virtual void Initialize() override;
		virtual void DrawGui() override;
		virtual const char* GetGuiName() const override;
		virtual ImGuiWindowFlags GetWindowFlags() const override;

		inline AetSet* GetAetSet() { return editorAetSet.get(); };
		inline SprSet* GetSprSet() { return sprSet.get(); };

	private:
		UniquePtr<AetCommandManager> commandManager;

		SpriteGetterFunction spriteGetterFunction;
		
		const bool asyncFileLoading = false;
		UniquePtr<FileLoader> sprSetFileLoader;

		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		struct
		{
			AetItemTypePtr selectedAetItem;
			AetItemTypePtr cameraSelectedAetItem;
		};

		UniquePtr<AetTreeView> treeView;
		UniquePtr<AetLayerView> layerView;
		UniquePtr<AetInspector> inspector;
		UniquePtr<AetTimeline> timeline;
		UniquePtr<AetRenderWindow> renderWindow;
		UniquePtr<AetHistoryWindow> historyWindow;

		struct
		{
			RefPtr<AetSet> editorAetSet;
			UniquePtr<SprSet> sprSet;
		};

		const char* debugAetPath = "dev_ram/aetset/aet_gam/aet_gam_cmn.bin";
		const char* debugSprPath = "dev_ram/sprset/spr_gam/spr_gam_cmn.bin";

		void UpdateFileLoading();

		void DrawAetSetLoader();
		void DrawSprSetLoader();

		bool LoadAetSet(const String& filePath);
		bool LoadSprSet(const String& filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();
	};
}
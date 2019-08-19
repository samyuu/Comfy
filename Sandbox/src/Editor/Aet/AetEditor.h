#pragma once
#include "Selection.h"
#include "AetTreeView.h"
#include "AetLayerView.h"
#include "AetInspector.h"
#include "Timeline/AetTimeline.h"
#include "AetRenderWindow.h"
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

		inline AetSet* GetAetSet() { return aetSet.get(); };
		inline SprSet* GetSprSet() { return sprSet.get(); };

	private:
		UniquePtr<AetCommandManager> commandManager;

		SpriteGetterFunction spriteGetterFunction;
		UniquePtr<FileLoader> sprSetFileLoader;

		Gui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		Gui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		UniquePtr<AetTreeView> treeView;
		UniquePtr<AetLayerView> layerView;
		UniquePtr<AetInspector> inspector;
		UniquePtr<AetTimeline> timeline;
		UniquePtr<AetRenderWindow> renderWindow;
		UniquePtr<AetHistoryWindow> historyWindow;

		struct
		{
			UniquePtr<AetSet> aetSet;
			UniquePtr<SprSet> sprSet;
		};

		Properties currentProperties;

		const char* debugAetPath = "dev_ram/aetset/aet_gam_cmn.bin";
		const char* debugSprPath = "dev_ram/sprset/spr_gam_cmn.bin";

		void UpdateFileLoading();

		void DrawAetSetLoader();
		void DrawSprSetLoader();

		bool LoadAetSet(const std::string& filePath);
		bool LoadSprSet(const std::string& filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();
	};
}
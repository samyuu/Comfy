#pragma once
#include "Selection.h"
#include "AetTreeView.h"
#include "AetLayerView.h"
#include "AetInspector.h"
#include "AetTimeline.h"
#include "AetRenderWindow.h"
#include "Editor/IEditorComponent.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/FileLoader.h"
#include "ImGui/Widgets/FileViewer.h"
#include <memory>

namespace Editor
{
	using namespace FileSystem;
	using namespace Auth2D;

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
		SpriteGetterFunction spriteGetterFunction;

		std::unique_ptr<FileLoader> sprSetFileLoader;

		ImGui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		ImGui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		std::unique_ptr<AetTreeView> treeView;
		std::unique_ptr<AetLayerView> layerView;
		std::unique_ptr<AetInspector> inspector;
		std::unique_ptr<AetTimeline> timeline;
		std::unique_ptr<AetRenderWindow> renderWindow;

		struct
		{
			std::unique_ptr<AetSet> aetSet;
			std::unique_ptr<SprSet> sprSet;
		};

		Properties currentProperties;

		const char* testAetPath = "dev_ram/aetset/aet_gam_cmn.bin";
		const char* testSprPath = "dev_ram/sprset/spr_gam_cmn.bin";

		void UpdateFileLoading();

		void DrawAetSetLoader();
		void DrawSprSetLoader();

		bool LoadAetSet(const std::string& filePath);
		bool LoadSprSet(const std::string& filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();
	};
}
#pragma once
#include "Selection.h"
#include "AetTreeView.h"
#include "AetInspector.h"
#include "AetTimeline.h"
#include "AetRenderWindow.h"
#include "Editor/IEditorComponent.h"
#include "FileSystem/Format/AetSet.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "ImGui/Widgets/FileViewer.h"
#include <memory>

namespace Editor
{
	using namespace FileSystem;

	class AetEditor : public IEditorComponent
	{
	public:
		AetEditor(Application* parent, PvEditor* editor);
		~AetEditor();

		virtual void Initialize() override;
		virtual void DrawGui() override;
		virtual const char* GetGuiName() const override;
		virtual ImGuiWindowFlags GetWindowFlags() const override;

		inline AetSet* GetAetSet() { return aetSet.get(); };
		inline SprSet* GetSprSet() { return sprSet.get(); };

	private:
		ImGui::FileViewer aetFileViewer = { "dev_ram/aetset/" };
		ImGui::FileViewer sprFileViewer = { "dev_ram/sprset/" };

		std::unique_ptr<AetTreeView> treeView;
		std::unique_ptr<AetInspector> inspector;
		std::unique_ptr<AetTimeline> timeline;
		std::unique_ptr<AetRenderWindow> renderWindow;

		struct
		{
			std::unique_ptr<AetSet> aetSet;
			std::unique_ptr<SprSet> sprSet;
		};

		Properties currentProperties;

		const char* testAetPath = "dev_ram/aetset/aet_tst000.bin";

		void DrawAetSetLoader();
		void DrawSprSetLoader();
		void DrawProperties();

		bool LoadAetSet(const std::string& filePath);
		bool LoadSprSet(const std::string& filePath);
		void OnAetSetLoaded();
		void OnSprSetLoaded();
	};
}
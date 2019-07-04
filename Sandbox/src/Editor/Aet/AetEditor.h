#pragma once
#include "Selection.h"
#include "AetTreeView.h"
#include "AetTimeline.h"
#include "AetRenderWindow.h"
#include "Editor/IEditorComponent.h"
#include "FileSystem/Format/AetSet.h"
#include "Graphics/Auth2D/AetMgr.h"
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

	private:
		ImGui::FileViewer fileViewer = { "dev_ram/aetset/" };

		std::unique_ptr<AetTreeView> treeView;
		std::unique_ptr<AetTimeline> timeline;
		std::unique_ptr<AetRenderWindow> renderWindow;

		struct
		{
			std::unique_ptr<AetSet> aetSet;
			//std::unique_ptr<SprSet> sprSet;
		};

		AetLyo* activeAetLyo;
		Properties currentProperties;

		AetItemTypePtr selected, hovered, lastHovered;

		const char* testAetPath = "dev_ram/aetset/aet_tst000.bin";

		void DrawInspectorAetObj(AetObj* aetObj);
		void DrawInspectorRegionData(AetRegion* spriteEntry);
		void DrawInspectorLayerData(AetLayer* aetLayer);
		void DrawInspectorAnimationData(AnimationData* animationData);
		void DrawKeyFrameProperties(KeyFrameProperties* properties);
		void DrawKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames);
		void DrawInspectorAetLayer(AetLayer* aetLayer);
		void DrawInspectorAetLyo(AetLyo* aetLyo);
		void DrawInspectorAetRegion(AetRegion* aetRegion);

		void DrawSetLoader();
		
		void DrawInspector();
		void DrawProperties();

		bool OpenAetSet(const std::string& filePath);
	};
}
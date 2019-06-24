#pragma once
#include "AetTimeline.h"
#include "AetRenderWindow.h"
#include "../IEditorComponent.h"
#include "../../BaseWindow.h"
#include "../../FileSystem/Format/AetSet.h"
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
		ImGui::FileViewer fileViewer{ "dev_ram/aetset/" };

		std::unique_ptr<AetTimeline> timeline;
		std::unique_ptr<AetRenderWindow> renderWindow;
		std::unique_ptr<AetSet> aetSet;

		enum class SelectionType
		{
			None,
			AetObj,
			AetLayer,
			AetLyo
		};

		AetLyo* activeAetLyo;

		struct AetItemTypePtr
		{
			SelectionType Type;
			union
			{
				void* ItemPtr;
				AetObj* AetObj;
				AetLyo* AetLyo;
				AetLayer* AetLayer;
			};
		};

		AetItemTypePtr selected;
		AetItemTypePtr lastHovered, hovered;

		struct
		{
			int newObjTypeIndex = AetObjType_Pic;
			char newObjNameBuffer[255];

			char aetSetPathBuffer[MAX_PATH] = "dev_ram/aetset/";
		};

		std::array<const char*, 4> aetObjTypeNames = { "nop", "pic", "aif", "eff" };

		const char* aetLayerContextMenuID = "AetLayerContextMenu";
		const char* addAetObjPopupID = "Add new AetObj";
		const char* testAetPath = "dev_ram/aetset/aet_tst000.bin";

		void SetSelectedItem(AetLyo* aetLyo, AetObj* value);
		void SetSelectedItem(AetLyo* aetLyo, AetLyo* value);
		void SetSelectedItem(AetLyo* aetLyo, AetLayer* value);
		void ResetSelectedItem();

		void DrawAetObj(AetObj* aetObj);
		void DrawRegionData(AetRegion* spriteEntry);
		void DrawLayerData(AetLayer* aetLayer);
		void DrawAnimationData(AnimationData* animationData);
		void DrawKeyFrameProperties(KeyFrameProperties* properties);
		void DrawKeyFrames(const char* name, std::vector<KeyFrame>* keyFrames);
		void DrawAetLayer(AetLayer* aetLayer);
		void DrawAetLyo(AetLyo* aetLyo);

		void DrawSetLoader();
		void DrawTreeView();
		void DrawInspector();
		void DrawProperties();

		bool OpenAetSet(const std::string& filePath);
	};
}
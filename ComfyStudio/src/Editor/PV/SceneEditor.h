#pragma once
#include "SceneGraph.h"
#include "SceneRenderWindow.h"
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Common/CameraController3D.h"
#include "Graphics/Direct3D/D3D_Renderer3D.h"
#include "Graphics/SprSet.h"
#include "ImGui/Widgets/FileViewer.h"
#include "StageTest.h"
#include "CharaTest.h"

namespace Editor
{
	class SceneEditor : public IEditorComponent
	{
	public:
		SceneEditor(Application* parent, EditorManager* editor);
		~SceneEditor() = default;

		void Initialize() override;
		void DrawGui() override;
		const char* GetGuiName() const override;
		ImGuiWindowFlags GetWindowFlags() const override;

	private:
		bool LoadRegisterObjSet(std::string_view objSetPath, std::string_view txpSetPath, EntityTag tag = 0);
		bool UnLoadUnRegisterObjSet(const Graphics::ObjSet* objSetToRemove);
		
		bool LoadStageObjects(StageType type, int id, int subID);
		bool UnLoadStageObjects();

	private:
		void DrawCameraGui();
		void DrawRenderingGui();
		void DrawFogGui();
		void DrawPostProcessingGui();
		void DrawLightGui();
		void DrawIBLGui();
		void DrawSceneGraphGui();
		void DrawEntityInspectorGui();
		void DrawObjectTestGui();
		void DrawStageTestGui();
		void DrawCharaTestGui();

	private:
		SceneGraph sceneGraph;

		Graphics::SceneContext context;
		CameraController3D cameraController;
		
		UniquePtr<Graphics::D3D_Renderer3D> renderer3D = nullptr;

		UniquePtr<SceneRenderWindow> renderWindow = nullptr;

		Gui::FileViewer objFileViewer = { "dev_rom/objset/" };

		StageTestData stageTestData;
		CharacterTestData charaTestData;

		struct EntityInspector
		{
			int EntityIndex = 0;
		} inspector;

		struct ObjTestData
		{
			int ObjSetIndex = 0;
			int ObjIndex = 0;
			int MaterialIndex = 0;
			int MeshIndex = 0;
		} objTestData;

		struct ObjSetResource
		{
			RefPtr<Graphics::ObjSet> ObjSet;
			EntityTag Tag;
		};

		std::vector<ObjSetResource> loadedObjSets;
	};
}
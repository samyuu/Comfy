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
#include "ExternalProcess.h"
#include <future>

namespace Comfy::Editor
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
		bool LoadRegisterObjSet(std::string_view objSetPath, std::string_view txpSetPath, EntityTag tag);
		bool UnLoadUnRegisterObjSet(const Graphics::ObjSet* objSetToRemove);
		
		bool LoadStageObjects(StageType type, int id, int subID);
		bool UnLoadStageObjects();

		enum class StageVisibilityType { None, All, GroundSky };
		void SetStageVisibility(StageVisibilityType visibility);

		enum EraseFlags
		{
			EraseFlags_Entities = (1 << 0),
			EraseFlags_ObjSets = (1 << 1),
		};
		void EraseByTag(EntityTag tag, EraseFlags flags);

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
		void DrawA3DTestGui();
		void DrawExternalProcessTestGui();
		void DrawDebugTestGui();

	private:
		void TakeSceneRenderTargetScreenshot(Graphics::D3D_RenderTarget& renderTarget);

	private:
		SceneGraph sceneGraph;

		Graphics::SceneContext context;
		CameraController3D cameraController;
		
		UniquePtr<Graphics::D3D_Renderer3D> renderer3D = nullptr;

		UniquePtr<SceneRenderWindow> renderWindow = nullptr;

		Gui::FileViewer objFileViewer = { "dev_rom/objset/" };

		StageTestData stageTestData;
		CharacterTestData charaTestData;
		
		struct ExternalProcessTest
		{
			ExternalProcess ExternalProcess;
			bool ShouldReadConfigFile = true;
			bool WasConfigInvalid = false;
			bool SyncReadCamera = false, SyncWriteCamera = false;
			bool SyncReadLightParam = false, SyncWriteLightParam = false;
		} externalProcessTest;

		// NOTE: To asyncronously take screenshots 
		std::future<void> lastScreenshotTaskFuture;

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
	};
}

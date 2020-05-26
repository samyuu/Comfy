#pragma once
#include "Types.h"
#include "SceneGraph.h"
#include "SceneRenderWindow.h"
#include "Editor/Core/IEditorComponent.h"
#include "Editor/Common/CameraController3D.h"
#include "Graphics/Auth2D/SprSet.h"
#include "ImGui/Widgets/FileViewer.h"
#include "StageTest.h"
#include "CharaTest.h"
#include "ExternalProcess.h"
#include "MaterialEditor.h"
#include <future>

namespace Comfy::Studio::Editor
{
	class SceneEditor : public IEditorComponent
	{
	public:
		SceneEditor(Application& parent, EditorManager& editor);
		~SceneEditor() = default;

	public:
		void OnFirstFrame() override;

	public:
		const char* GetName() const override;
		ImGuiWindowFlags GetFlags() const override;
		void Gui() override;

	private:
		bool LoadRegisterObjSet(std::string_view objSetPath, std::string_view texSetPath, EntityTag tag);
		bool UnLoadUnRegisterObjSet(const Graphics::ObjSet* objSetToRemove);

		bool LoadStageObjects(StageType type, int id, int subID, bool loadLightParam = true);
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
		struct ViewportContext
		{
			Render::PerspectiveCamera Camera;
			CameraController3D CameraController;
			std::unique_ptr<SceneRenderWindow> RenderWindow = nullptr;
			bool IsOpen = true;
		};

		void AddViewport(ViewportContext* baseViewport);
		ViewportContext& FindActiveViewport();

	private:
		void DrawCameraGui(ViewportContext& activeViewport);
		void DrawObjSetLoaderGui();
		void DrawRenderingGui(ViewportContext& activeViewport);
		void DrawFogGui();
		void DrawPostProcessingGui();
		void DrawLightGui();
		void DrawIBLGui();
		void DrawSceneGraphGui(ViewportContext& activeViewport);
		void DrawEntityInspectorGui();
		void DrawObjectTestGui();
		void DrawStageTestGui();
		void DrawCharaTestGui();
		void DrawA3DTestGui(ViewportContext& activeViewport);
		void DrawExternalProcessTestGui(ViewportContext& activeViewport);
		void DrawDebugTestGui(ViewportContext& activeViewport);

	private:
		void TakeScreenshotGui(ViewportContext& activeViewport);
		void TakeSceneRenderTargetScreenshot(Render::RenderTarget3D& renderTarget);

	private:
		SceneGraph sceneGraph;

		Render::SceneParam3D scene;
		std::unique_ptr<Render::Renderer3D> renderer3D = nullptr;

		// NOTE: Store additional viewports as unique_ptrs to never invalidate their camera / controller references
		ViewportContext mainViewport;
		std::vector<std::unique_ptr<ViewportContext>> additionalViewports;

		Gui::FileViewer objSetFileViewer = { "dev_rom/objset/" };
		Graphics::Transform objSetTransform = Graphics::Transform(vec3(0.0f));

		StageTestData stageTestData;
		CharacterTestData charaTestData;
		MaterialEditor materialEditor;

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

		std::array<char, 260> lightPathBuffer = { "dev_rom/light_param/light_tst.txt" };
		std::array<char, 260> glowPathBuffer = { "dev_rom/light_param/glow_tst.txt" };
		std::array<char, 260> fogPathBuffer = { "dev_rom/light_param/fog_tst.txt" };
		std::array<char, 260> iblPathBuffer = { "dev_rom/ibl/tst.ibl" };

		// NOTE: To keep track of the open and closed render target debug windows
		u32 openRenderTargetsFlags = 0;

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

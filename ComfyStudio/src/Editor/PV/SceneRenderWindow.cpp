#include "SceneRenderWindow.h"
#include "FileSystem/FileHelper.h"
#include "Input/KeyCode.h"
#include "Core/ComfyData.h"
#include "Core/TimeSpan.h"
#include "Misc/StringHelper.h"
#include <FontIcons.h>

namespace Editor
{
	using namespace Graphics;

	SceneRenderWindow::SceneRenderWindow(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		renderer3D = MakeUnique<D3D_Renderer3D>();
	}

	SceneRenderWindow::~SceneRenderWindow()
	{
	}

	const char* SceneRenderWindow::GetGuiName() const
	{
		return "Scene Window";
	}

	void SceneRenderWindow::Initialize()
	{
		RenderWindowBase::Initialize();

//#define OBJ_FILE "stgtst007"
//#define OBJ_FILE "cmnitm1001"
//#define OBJ_FILE "rinitm000"
//#define OBJ_FILE "rinitm001"
//#define OBJ_FILE "rinitm047"
//#define OBJ_FILE "rinitm301"
//#define OBJ_FILE "rinitm532"
#define OBJ_FILE "stgns006"
//#define OBJ_FILE "stgd2ns036"
//#define OBJ_FILE "stgns008"
//#define OBJ_FILE "dbg"

		LoadObjSet("dev_rom/objset/" OBJ_FILE "/" OBJ_FILE "_obj.bin");
	}

	void SceneRenderWindow::DrawGui()
	{
		RenderWindowBase::DrawGui();

		// NOTE: Push / Pop reset the StyleVar for the ComfyDebug window
		OnWindowEnd();
		{
			if (Gui::Begin("Comfy Debug"))
				DrawComfyDebugGui();
			Gui::End();

			if (Gui::Begin(ICON_FA_FOLDER "  ObjSet Loader"))
			{
				Gui::BeginChild("ObjSetLoaderChild");
				if (objFileViewer.DrawGui())
					LoadObjSet(objFileViewer.GetFileToOpen());
				Gui::EndChild();
			}
			Gui::End();
		}
		OnWindowBegin();
	}

	void SceneRenderWindow::PostDrawGui()
	{
		if (objSet == nullptr)
			return;

		// auto& obj = objSet->at(objectIndex);
		// Gui::Text("%s (%d/%d)", obj.Name.c_str(), objectIndex, objSet->size());
	}

	void SceneRenderWindow::OnWindowBegin()
	{
		RenderWindowBase::PushWindowPadding();
	}

	void SceneRenderWindow::OnWindowEnd()
	{
		RenderWindowBase::PopWindowPadding();
	}

	void SceneRenderWindow::DrawComfyDebugGui()
	{
		if (Gui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::Text("Camera");
			Gui::DragFloat("Field Of View", &sceneData.Camera.FieldOfView, 1.0f, 1.0f, 180.0f);
			Gui::DragFloat("Near Plane", &sceneData.Camera.NearPlane, 0.001f, 0.001f, 1.0f);
			Gui::DragFloat("Far Plane", &sceneData.Camera.FarPlane);
			Gui::DragFloat3("Position", glm::value_ptr(sceneData.Camera.Position), 0.01f);
			Gui::DragFloat3("Target", glm::value_ptr(sceneData.Camera.Target), 0.01f);
			Gui::DragFloat("Smoothness", &sceneData.CameraSmoothness, 1.0f, 0.0f, 250.0f);

			Gui::Text("Camera Rotation");
			Gui::DragFloat("Pitch", &sceneData.TargetCameraPitch, 1.0f);
			Gui::DragFloat("Yaw", &sceneData.TargetCameraYaw, 1.0f);
		}

		if (Gui::CollapsingHeader("Object Test", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::Checkbox("Wireframe", &renderer3D->DEBUG_RenderWireframe);
			Gui::Checkbox("Alpha Sort", &renderer3D->DEBUG_AlphaSort);
			Gui::Checkbox("Render Opaque", &renderer3D->DEBUG_RenderOpaque);
			Gui::Checkbox("Render Transparent", &renderer3D->DEBUG_RenderTransparent);

			Gui::BeginChild("ObjSelectionChild", vec2(0, 220), true);

			if (Gui::Selectable("All Objects", (objectIndex < 0)))
				objectIndex = -1;

			if (objSet != nullptr)
			{
				for (int i = 0; i < static_cast<int>(objSet->size()); i++)
				{
					if (Gui::Selectable(objSet->at(i).Name.c_str(), i == objectIndex))
						objectIndex = i;
				}
			}

			Gui::EndChild();
		}

		if (Gui::CollapsingHeader("Post Processing"))
		{
			Gui::DragFloat("Saturation", &postProcessData.Saturation, 0.015f, 1.0f, 5.0f);
			Gui::DragFloat("Brightness", &postProcessData.Brightness, 0.015f, 0.1f, 5.0f);
		}
	}

	void SceneRenderWindow::LoadObjSet(const std::string& filePath)
	{
		if (!EndsWith(filePath, "_obj.bin"))
			return;

		auto directory = FileSystem::GetDirectory(filePath);
		auto fileName = FileSystem::GetFileName(filePath, false);
		std::string txpPath = directory + "/" + fileName.substr(0, fileName.size() - strlen("_obj")) + "_tex.bin";

		if (!FileSystem::FileExists(txpPath))
			return;

		objSet = MakeUnique<ObjSet>();
		objSet->Load(filePath);
		objSet->UploadAll();

		std::vector<uint8_t> txpFileContent;
		FileSystem::FileReader::ReadEntireFile(txpPath, &txpFileContent);

		objSet->TxpSet = MakeUnique<TxpSet>();
		objSet->TxpSet->Parse(txpFileContent.data());
		objSet->TxpSet->UploadAll(nullptr);
	}

	void SceneRenderWindow::OnUpdateInput()
	{
		return;
	}

	void SceneRenderWindow::OnUpdate()
	{
		UpdateCamera();
	}

	void SceneRenderWindow::UpdateCamera()
	{
		ImGuiIO& io = Gui::GetIO();

		vec3 front;
		front.x = cos(glm::radians(sceneData.CameraYaw)) * cos(glm::radians(sceneData.CameraPitch));
		front.y = sin(glm::radians(sceneData.CameraPitch));
		front.z = sin(glm::radians(sceneData.CameraYaw)) * cos(glm::radians(sceneData.CameraPitch));

		const bool fastCamera = Gui::IsKeyDown(KeyCode_Shift);
		const bool slowCamera = Gui::IsKeyDown(KeyCode_Alt);

		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 25.5f : 2.25f)) * io.DeltaTime;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsMouseDown(0))
			{
				sceneData.TargetCameraYaw += io.MouseDelta.x * sceneData.CameraSensitivity;
				sceneData.TargetCameraPitch -= io.MouseDelta.y * sceneData.CameraSensitivity;
			}

			if (Gui::IsWindowHovered())
			{
				const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);

				if (io.MouseWheel > 0)
					sceneData.Camera.Position += front * scrollStep;
				if (io.MouseWheel < 0)
					sceneData.Camera.Position -= front * scrollStep;
			}
		}

		if (sceneData.TargetCameraPitch > +89.0f) sceneData.TargetCameraPitch = +89.0f;
		if (sceneData.TargetCameraPitch < -89.0f) sceneData.TargetCameraPitch = -89.0f;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsKeyDown(KeyCode_W))
				sceneData.Camera.Position += front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_S))
				sceneData.Camera.Position -= front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_A))
				sceneData.Camera.Position -= glm::normalize(glm::cross(front, sceneData.Camera.UpDirection)) * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_D))
				sceneData.Camera.Position += glm::normalize(glm::cross(front, sceneData.Camera.UpDirection)) * cameraSpeed;

			if (Gui::IsKeyDown(KeyCode_Space))
				sceneData.Camera.Position += sceneData.Camera.UpDirection * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_Control))
				sceneData.Camera.Position -= sceneData.Camera.UpDirection * cameraSpeed;
		}

		sceneData.Camera.Target = sceneData.Camera.Position + glm::normalize(front);

		if (sceneData.CameraSmoothness > 0.0f)
		{
			sceneData.CameraYaw = ImLerp(sceneData.CameraYaw, sceneData.TargetCameraYaw, io.DeltaTime * sceneData.CameraSmoothness);
			sceneData.CameraPitch = ImLerp(sceneData.CameraPitch, sceneData.TargetCameraPitch, io.DeltaTime * sceneData.CameraSmoothness);
		}
		else
		{
			sceneData.CameraYaw = sceneData.TargetCameraYaw;
			sceneData.CameraPitch = sceneData.TargetCameraPitch;
		}

		sceneData.Camera.UpdateMatrices();
	}

	void SceneRenderWindow::OnRender()
	{
		if (objSet == nullptr)
			return;

		renderTarget->Bind();
		{
			D3D.SetViewport(renderTarget->GetSize());
			renderTarget->Clear(GetColorVec4(EditorColor_BaseClear));

			renderer3D->Begin(sceneData.Camera);
			{
				if (objectIndex < 0)
				{
					for (auto& obj : *objSet)
						renderer3D->Draw(objSet.get(), &obj, vec3(0.0f, 0.0f, 0.0f));
				}
				else if (objectIndex < objSet->size() && objSet->size() != 0)
				{
					renderer3D->Draw(objSet.get(), objSet->GetObjAt(objectIndex), vec3(0.0f, 0.0f, 0.0f));
				}
			}
			renderer3D->End();
		}
		renderTarget->UnBind();
	}

	void SceneRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		vec2 renderRegionSize = GetRenderRegion().GetSize();
		sceneData.Camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		// postProcessingData.postProcessingRenderTarget.Resize(size);
	}
}

#include "SceneRenderWindow.h"
#include "FileSystem/FileHelper.h"
#include "Input/KeyCode.h"
#include "Misc/StringHelper.h"
#include <FontIcons.h>

namespace Editor
{
	using namespace Graphics;

	template <typename T>
	void LoadParseUploadLightParamFile(std::string_view filePath, T& param)
	{
		if (!FileSystem::FileExists(filePath))
			return;

		std::vector<uint8_t> fileContent;
		FileSystem::FileReader::ReadEntireFile(filePath, &fileContent);

		param.Parse(fileContent.data());

		if constexpr (std::is_same<T, LightDataIBL>::value)
			param.UploadAll();
	}

	SceneRenderWindow::SceneRenderWindow(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		renderer3D = MakeUnique<D3D_Renderer3D>();

		struct LightParamPaths
		{
			std::string_view Glow, Light, IBL;
		} paths = { "dev_rom/light_param/glow_tst.txt", "dev_rom/light_param/light_tst.txt", "dev_rom/ibl/tst007.ibl" };

		LoadParseUploadLightParamFile(paths.Glow, context.Glow);
		LoadParseUploadLightParamFile(paths.Light, context.Light);
		LoadParseUploadLightParamFile(paths.IBL, context.IBL);


#if 0 // DEBUG:
		context.Camera.FieldOfView = 70.0f;
		context.Camera.Position = vec3(-1.0f, 0.5f, 1.5f);
		cameraController.TargetCameraPitch = -5.0f;
		cameraController.TargetCameraYaw = -50.0f;
#endif
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
		context.OutputRenderTarget = renderTarget.get();

//#define OBJ_FILE "f_stgtst004"
#define OBJ_FILE "stgtst007"
//#define OBJ_FILE "cmnitm1001"
//#define OBJ_FILE "rinitm000"
//#define OBJ_FILE "rinitm001"
//#define OBJ_FILE "rinitm047"
//#define OBJ_FILE "rinitm301"
//#define OBJ_FILE "rinitm532"
//#define OBJ_FILE "stgns006"
//#define OBJ_FILE "stgd2ns036"
//#define OBJ_FILE "stgns008"
//#define OBJ_FILE "dbg"

#ifdef OBJ_FILE
		if (objSet == nullptr)
			LoadObjSet("dev_rom/objset/" OBJ_FILE "/" OBJ_FILE "_obj.bin");
#endif
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
		if (Gui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_None))
		{
			auto& camera = context.Camera;

			Gui::PushID(&camera);
			Gui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 180.0f);
			Gui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
			Gui::DragFloat("Far Plane", &camera.FarPlane);
			Gui::DragFloat3("Position", glm::value_ptr(camera.Position), 0.01f);
			Gui::DragFloat3("Target", glm::value_ptr(camera.Target), 0.01f);
			Gui::DragFloat("Smoothness", &cameraController.CameraSmoothness, 1.0f, 0.0f, 250.0f);

			Gui::DragFloat("Pitch", &cameraController.TargetCameraPitch, 1.0f);
			Gui::DragFloat("Yaw", &cameraController.TargetCameraYaw, 1.0f);
			Gui::PopID();
		}

		if (Gui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_None))
		{
			Gui::PushID(&context.RenderParameters);
			Gui::CheckboxFlags("DebugFlags_0", &context.RenderParameters.DebugFlags, (1 << 0));
			Gui::CheckboxFlags("DebugFlags_1", &context.RenderParameters.DebugFlags, (1 << 1));
			Gui::CheckboxFlags("DebugFlags_2", &context.RenderParameters.DebugFlags, (1 << 2));
			Gui::Checkbox("Clear", &context.RenderParameters.Clear);
			Gui::Checkbox("Wireframe", &context.RenderParameters.Wireframe);
			Gui::Checkbox("Wireframe Overlay", &context.RenderParameters.WireframeOverlay);
			Gui::Checkbox("Alpha Sort", &context.RenderParameters.AlphaSort);
			Gui::Checkbox("Render Opaque", &context.RenderParameters.RenderOpaque);
			Gui::Checkbox("Render Transparent", &context.RenderParameters.RenderTransparent);
			Gui::SliderInt("Anistropic Filtering", &context.RenderParameters.AnistropicFiltering, D3D11_MIN_MAXANISOTROPY, D3D11_MAX_MAXANISOTROPY);

			if (Gui::CollapsingHeader("Resolution"))
			{
				auto clampSize = [](ivec2 size) { return glm::clamp(size, ivec2(1, 1), ivec2(16384, 16384)); };

				static ivec2 customResolution = { 1280, 720 };

				if (Gui::InputInt2("Custom Resolution", glm::value_ptr(customResolution)))
					customResolution = clampSize(customResolution);

				if (Gui::Button("Render Region x1")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 1.0f));
				if (Gui::Button("Render Region x2")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 2.0f));
				if (Gui::Button("Render Region x4")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 4.0f));
				if (Gui::Button("Render Region x8")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 8.0f));
				if (Gui::Button("Render Region x16")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 16.0f));
				if (Gui::Button("Render Region x32")) customResolution = (clampSize(vec2(GetRenderRegion().GetSize()) * 32.0f));

				if (Gui::Button("Resize Custom"))
					context.Resize(customResolution);
			}

			Gui::PopID();
		}

		if (Gui::CollapsingHeader("Glow", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::PushID(&context.Glow);
			Gui::SliderFloat("Exposure", &context.Glow.Exposure, 0.0f, 4.0f);
			Gui::SliderFloat("Gamma", &context.Glow.Gamma, 0.2f, 2.2f);
			Gui::SliderInt("Saturate Power", &context.Glow.SaturatePower, 1, 6);
			Gui::SliderFloat("Saturate Coefficient", &context.Glow.SaturateCoefficient, 0.0f, 1.0f);
			Gui::PopID();
		}

		if (Gui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto lightGui = [](Light& light)
			{
				Gui::ColorEdit3("Ambient", glm::value_ptr(light.Ambient), ImGuiColorEditFlags_Float);
				Gui::ColorEdit3("Diffuse", glm::value_ptr(light.Diffuse), ImGuiColorEditFlags_Float);
				Gui::ColorEdit3("Specular", glm::value_ptr(light.Specular), ImGuiColorEditFlags_Float);
				Gui::DragFloat3("Position", glm::value_ptr(light.Position), 0.01f);
				Gui::PopID();
			};
			
			if (Gui::CollapsingHeader("IBL Light"))
			{
				Gui::PushID(&context.IBL);
				Gui::ColorEdit3("Chara Light Color", glm::value_ptr(context.IBL.Character.LightColor), ImGuiColorEditFlags_Float);
				Gui::ColorEdit3("Stage Light Color", glm::value_ptr(context.IBL.Stage.LightColor), ImGuiColorEditFlags_Float);
				Gui::PopID();
			}

		}

		if (Gui::CollapsingHeader("Object Test", ImGuiTreeNodeFlags_None))
		{
			Gui::BeginChild("ObjSelectionChild", vec2(0.0f, 140.0f), true);

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

		if (Gui::CollapsingHeader("Material Test", ImGuiTreeNodeFlags_None))
		{
			if (objSet != nullptr)
			{
				Gui::PushID(objSet.get());

				auto getMaterialObjName = [&](int index) { return (index < 0 || index > objSet->size()) ? "None" : objSet->GetObjAt(index)->Name.c_str(); };
				auto getMaterialName = [](Obj* obj, int index) { return (obj == nullptr || index < 0 || index > obj->Materials.size()) ? "None" : obj->Materials[index].Name; };

				if (Gui::BeginCombo("Object", getMaterialObjName(materialObjIndex), ImGuiComboFlags_HeightLarge))
				{
					for (int objIndex = 0; objIndex < static_cast<int>(objSet->size()); objIndex++)
					{
						if (Gui::Selectable(getMaterialObjName(objIndex), (objIndex == materialObjIndex)))
							materialObjIndex = objIndex;

						if (objIndex == materialObjIndex)
							Gui::SetItemDefaultFocus();
					}

					Gui::ComfyEndCombo();
				}

				Obj* matObj = (materialObjIndex >= 0 && materialObjIndex < objSet->size()) ? objSet->GetObjAt(materialObjIndex) : nullptr;
				if (Gui::BeginCombo("Material", getMaterialName(matObj, materialIndex), ImGuiComboFlags_HeightLarge))
				{
					if (matObj != nullptr)
					{
						for (int matIndex = 0; matIndex < static_cast<int>(matObj->Materials.size()); matIndex++)
						{
							if (Gui::Selectable(getMaterialName(matObj, matIndex), (matIndex == materialIndex)))
								materialIndex = matIndex;

							if (matIndex == materialIndex)
								Gui::SetItemDefaultFocus();
						}
					}

					Gui::EndCombo();
				}

				Material* material = (matObj != nullptr&& materialIndex >= 0 && materialIndex < matObj->Materials.size()) ? &matObj->Materials[materialIndex] : nullptr;

				if (material != nullptr)
				{
					Gui::ColorEdit3("Diffuse", glm::value_ptr(material->DiffuseColor), ImGuiColorEditFlags_Float);
					Gui::DragFloat("Transparency", &material->Transparency);
					Gui::ColorEdit3("Specular", glm::value_ptr(material->SpecularColor), ImGuiColorEditFlags_Float);
					Gui::DragFloat("Reflectivity", &material->Reflectivity);
					Gui::DragFloat("Shininess", &material->Shininess);
					Gui::ColorEdit3("Ambient", glm::value_ptr(material->AmbientColor), ImGuiColorEditFlags_Float);
					Gui::ColorEdit3("Emission", glm::value_ptr(material->EmissionColor), ImGuiColorEditFlags_Float);
					Gui::InputText("Shader", material->Shader.data(), material->Shader.size(), ImGuiInputTextFlags_ReadOnly);
				}

				Gui::PopID();
			}
		}
	}

	void SceneRenderWindow::LoadObjSet(const std::string& filePath)
	{
		if (!EndsWith(filePath, "_obj.bin"))
			return;

		auto directory = FileSystem::GetDirectory(filePath);
		auto fileName = FileSystem::GetFileName(filePath, false);

		std::string txpPath;
		txpPath.append(directory);
		txpPath.append("/");
		txpPath.append(fileName.substr(0, fileName.size() - strlen("_obj")));
		txpPath.append("_tex.bin");

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
		front.x = cos(glm::radians(cameraController.CameraYaw)) * cos(glm::radians(cameraController.CameraPitch));
		front.y = sin(glm::radians(cameraController.CameraPitch));
		front.z = sin(glm::radians(cameraController.CameraYaw)) * cos(glm::radians(cameraController.CameraPitch));

		const bool fastCamera = Gui::IsKeyDown(KeyCode_Shift);
		const bool slowCamera = Gui::IsKeyDown(KeyCode_Alt);

		const float cameraSpeed = (slowCamera ? 0.25f : (fastCamera ? 25.5f : 2.25f)) * io.DeltaTime;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsMouseDown(0))
			{
				cameraController.TargetCameraYaw += io.MouseDelta.x * cameraController.CameraSensitivity;
				cameraController.TargetCameraPitch -= io.MouseDelta.y * cameraController.CameraSensitivity;
			}

			if (Gui::IsWindowHovered())
			{
				const float scrollStep = slowCamera ? 0.5f : (fastCamera ? 12.5f : 1.5f);

				if (io.MouseWheel > 0)
					context.Camera.Position += front * scrollStep;
				if (io.MouseWheel < 0)
					context.Camera.Position -= front * scrollStep;
			}
		}

		if (cameraController.TargetCameraPitch > +89.0f) cameraController.TargetCameraPitch = +89.0f;
		if (cameraController.TargetCameraPitch < -89.0f) cameraController.TargetCameraPitch = -89.0f;

		if (Gui::IsWindowFocused())
		{
			if (Gui::IsKeyDown(KeyCode_W))
				context.Camera.Position += front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_S))
				context.Camera.Position -= front * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_A))
				context.Camera.Position -= glm::normalize(glm::cross(front, context.Camera.UpDirection)) * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_D))
				context.Camera.Position += glm::normalize(glm::cross(front, context.Camera.UpDirection)) * cameraSpeed;

			if (Gui::IsKeyDown(KeyCode_Space))
				context.Camera.Position += context.Camera.UpDirection * cameraSpeed;
			if (Gui::IsKeyDown(KeyCode_Control))
				context.Camera.Position -= context.Camera.UpDirection * cameraSpeed;
		}

		context.Camera.Target = context.Camera.Position + glm::normalize(front);

		if (cameraController.CameraSmoothness > 0.0f)
		{
			cameraController.CameraYaw = ImLerp(cameraController.CameraYaw, cameraController.TargetCameraYaw, io.DeltaTime * cameraController.CameraSmoothness);
			cameraController.CameraPitch = ImLerp(cameraController.CameraPitch, cameraController.TargetCameraPitch, io.DeltaTime * cameraController.CameraSmoothness);
		}
		else
		{
			cameraController.CameraYaw = cameraController.TargetCameraYaw;
			cameraController.CameraPitch = cameraController.TargetCameraPitch;
		}

		context.Camera.UpdateMatrices();
	}

	void SceneRenderWindow::OnRender()
	{
		if (objSet == nullptr)
			return;

		renderTarget->Bind();
		{
			D3D.SetViewport(renderTarget->GetSize());
			context.RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

			renderer3D->Begin(context);
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
		context.Camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		context.Resize(size);
	}
}

#include "SceneRenderWindow.h"
#include "FileSystem/FileHelper.h"
#include "Input/KeyCode.h"
#include "Misc/StringHelper.h"
#include <FontIcons.h>

namespace Editor
{
	using namespace Graphics;

	namespace
	{
		template <typename T>
		bool LoadParseUploadLightParamFile(std::string_view filePath, T& param)
		{
			if (!FileSystem::FileExists(filePath))
				return false;

			std::vector<uint8_t> fileContent;
			FileSystem::FileReader::ReadEntireFile(filePath, &fileContent);

			param.Parse(fileContent.data());

			if constexpr (std::is_same<T, LightDataIBL>::value)
				param.UploadAll();

			return true;
		}

		enum StageType { STGTST, STGNS, STGD2NS, STGPV };

		void LoadStageLightParamFiles(SceneContext& context, StageType stageType, int stageID)
		{
			constexpr std::array formatStrings = { "tst%03d", "ns%03d", "d2ns%03d", "pv%03ds01" };
			const char* formatString = (stageID == 0 && stageType == StageType::STGTST) ? "tst" : formatStrings[static_cast<size_t>(stageType)];

			std::array<char, MAX_PATH> fileName;
			sprintf_s(fileName.data(), fileName.size(), formatString, stageID);

			const char* romDirectory = "dev_rom";
			std::array<char, MAX_PATH> pathBuffer;

			sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/light_param/glow_%s.txt", romDirectory, fileName.data());
			if (!LoadParseUploadLightParamFile(pathBuffer.data(), context.Glow))
				LoadParseUploadLightParamFile("dev_rom/light_param/glow_tst.txt", context.Glow);

			sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/light_param/light_%s.txt", romDirectory, fileName.data());
			if (!LoadParseUploadLightParamFile(pathBuffer.data(), context.Light))
				LoadParseUploadLightParamFile("dev_rom/light_param/light_tst.txt", context.Light);

			sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/ibl/%s.ibl", romDirectory, fileName.data());
			if (!LoadParseUploadLightParamFile(pathBuffer.data(), context.IBL))
				LoadParseUploadLightParamFile("dev_rom/ibl/tst.ibl", context.IBL);
		}

		template <typename T>
		bool LoadStage(SceneContext& context, StageType stageType, int stageID, UniquePtr<ObjSet>& textureObjSet, T objSetLoaderFunc, D3D_Renderer3D& renderer)
		{
			LoadStageLightParamFiles(context, stageType, stageID);

			if (stageType == StageType::STGPV)
			{
				constexpr const char* directory = "dev_rom/objset/stgpv";
				char pathBuffer[MAX_PATH];

				sprintf_s(pathBuffer, "%s/stgpv%03ds01/stgpv%03ds01_obj.bin", directory, stageID, stageID);
				if (!FileSystem::FileExists(pathBuffer))
					return false;

				objSetLoaderFunc(pathBuffer);

				sprintf_s(pathBuffer, "%s/stgpv%03d/stgpv%03d_obj.bin", directory, stageID, stageID);
				textureObjSet = ObjSet::MakeUniqueReadParseUpload(pathBuffer);

				sprintf_s(pathBuffer, "%s/stgpv%03d/stgpv%03d_tex.bin", directory, stageID, stageID);
				textureObjSet->TxpSet = TxpSet::MakeUniqueReadParseUpload(pathBuffer, textureObjSet.get());
			}
			else
			{
				// TODO: ...
			}

			if (textureObjSet->TxpSet != nullptr)
				renderer.RegisterTextureIDs(*textureObjSet->TxpSet);

			return true;
		}

		int FindGroundObj(ObjSet* objSet)
		{
			if (objSet == nullptr)
				return -1;

			auto result = std::find_if(objSet->begin(), objSet->end(), [](auto& obj) { return EndsWithInsensitive(obj.Name, "_gnd"); });
			return (result == objSet->end()) ? -1 : static_cast<int>(std::distance(objSet->begin(), result));
		}
	}

	SceneRenderWindow::SceneRenderWindow(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		renderer3D = MakeUnique<D3D_Renderer3D>();



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
#define OBJ_FILE "stgns006"

#ifdef OBJ_FILE
		if (objSet == nullptr)
		{
			LoadObjSet("dev_rom/objset/" OBJ_FILE "/" OBJ_FILE "_obj.bin");
			LoadStageLightParamFiles(context, StageType::STGNS, 6);
		}
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

		Gui::DEBUG_NOSAVE_WINDOW("STGPV", [&]()
		{
			static int stgID = 0;

			if (Gui::SliderInt("STGPV_ID", &stgID, 1, 999, "STGPV_%03d"))
			{
				LoadStage(context, StageType::STGPV, stgID, textureObjSet, [&](auto path) { LoadObjSet(path); }, *renderer3D);
				objectIndex = FindGroundObj(objSet.get());
			}
		});
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
			Gui::Checkbox("Render Reflection", &context.RenderParameters.RenderReflection);
			Gui::Checkbox("Render Opaque", &context.RenderParameters.RenderOpaque);
			Gui::Checkbox("Render Transparent", &context.RenderParameters.RenderTransparent);
			Gui::SliderInt("Anistropic Filtering", &context.RenderParameters.AnistropicFiltering, D3D11_MIN_MAXANISOTROPY, D3D11_MAX_MAXANISOTROPY);

			if (Gui::CollapsingHeader("Resolution"))
			{
				auto clampSize = [](ivec2 size) { return glm::clamp(size, ivec2(1, 1), ivec2(16384, 16384)); };

				constexpr std::array namedFactors =
				{
					std::make_pair("Render Region x1", 1.0f),
					std::make_pair("Render Region x2", 2.0f),
					std::make_pair("Render Region x4", 4.0f),
					std::make_pair("Render Region x8", 8.0f),
					std::make_pair("Render Region x16", 16.0f),
				};

				ivec2 renderResolution = context.RenderTarget.GetSize();
				Gui::InputInt2("Render Resolution", glm::value_ptr(renderResolution));
				Gui::ItemContextMenu("RenderResolutionContextMenu", [&]()
				{
					Gui::Text("Set Render Resolution:");
					Gui::Separator();
					for (auto[name, factor] : namedFactors)
						if (Gui::MenuItem(name)) renderResolution = ivec2(vec2(GetRenderRegion().GetSize()) * factor);
				});

				if (renderResolution != context.RenderTarget.GetSize())
					context.Resize(clampSize(renderResolution));

				ivec2 reflectionResolution = context.RenderParameters.ReflectionResolution;
				Gui::InputInt2("Reflection Resolution", glm::value_ptr(reflectionResolution));
				Gui::ItemContextMenu("ReflectionResolutionContextMenu", [&]()
				{
					Gui::Text("Set Reflection Resolution:");
					Gui::Separator();
					if (Gui::MenuItem("256x256")) reflectionResolution = ivec2(256, 256);
					if (Gui::MenuItem("512x512")) reflectionResolution = ivec2(512, 512);

					for (auto[name, factor] : namedFactors)
						if (Gui::MenuItem(name)) reflectionResolution = (vec2(GetRenderRegion().GetSize()) * factor);
				});

				if (reflectionResolution != context.RenderParameters.ReflectionResolution)
					context.RenderParameters.ReflectionResolution = clampSize(reflectionResolution);
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
				Gui::PushID(&light);
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

			if (Gui::CollapsingHeader("Character Light"))
				lightGui(context.Light.Character);
			if (Gui::CollapsingHeader("Stage Light"))
				lightGui(context.Light.Stage);
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

		objSet = ObjSet::MakeUniqueReadParseUpload(filePath);
		objSet->TxpSet = TxpSet::MakeUniqueReadParseUpload(txpPath, objSet.get());

		renderer3D->ClearTextureIDs();
		renderer3D->RegisterTextureIDs(*objSet->TxpSet);
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

		renderTarget->BindSetViewport();
		{
			context.RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

			renderer3D->Begin(context);
			{
				if (objectIndex < 0)
				{
					for (auto& obj : *objSet)
						renderer3D->Draw(&obj, vec3(0.0f, 0.0f, 0.0f));
				}
				else if (objectIndex < objSet->size() && objSet->size() != 0)
				{
					renderer3D->Draw(objSet->GetObjAt(objectIndex), vec3(0.0f, 0.0f, 0.0f));
				}

				// DEBUG:
				for (auto& obj : *objSet)
				{
					if (EndsWithInsensitive(obj.Name, "_reflect"))
						renderer3D->DrawReflection(&obj, vec3(0.0f, 0.0f, 0.0f));
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

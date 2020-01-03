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

			param.Parse(fileContent.data(), fileContent.size());

			if constexpr (std::is_same<T, LightDataIBL>::value)
				param.UploadAll();

			return true;
		}

		void LoadStageLightParamFiles(SceneContext& context, StageType stageType, int stageID, int stageSubID = 0)
		{
			constexpr std::array formatStrings = { "tst%03d", "ns%03d", "d2ns%03d", "pv%03ds%02d" };
			const char* formatString = (stageID == 0 && stageType == StageType::STGTST) ? "tst" : formatStrings[static_cast<size_t>(stageType)];

			std::array<char, MAX_PATH> fileName;
			sprintf_s(fileName.data(), fileName.size(), formatString, stageID, stageSubID);

			const char* romDirectory = "dev_rom";
			std::array<char, MAX_PATH> pathBuffer;

			sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/light_param/fog_%s.txt", romDirectory, fileName.data());
			if (!LoadParseUploadLightParamFile(pathBuffer.data(), context.Fog))
				LoadParseUploadLightParamFile("dev_rom/light_param/fog_tst.txt", context.Fog);

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
		bool LoadStageObj(SceneContext& context, StageType stageType, int stageID, int stageSubID, UniquePtr<ObjSet>& textureObjSet, T objSetLoaderFunc, D3D_Renderer3D& renderer)
		{
			constexpr const char* objSetDirectory = "dev_rom/objset";
			std::array<char, MAX_PATH> pathBuffer;

			if (textureObjSet != nullptr && textureObjSet->TxpSet != nullptr)
				renderer.UnRegisterTextureIDs(*textureObjSet->TxpSet);

			textureObjSet = nullptr;

			constexpr std::array formatStrings = { "stgtst%03d", "stgns%03d", "stgd2ns%03d", "stgpv%03ds%02d" };
			const char* formatString = (stageID == 0 && stageType == StageType::STGTST) ? "tst" : formatStrings[static_cast<size_t>(stageType)];

			std::array<char, MAX_PATH> fileName;
			sprintf_s(fileName.data(), fileName.size(), formatString, stageID, stageSubID);

			constexpr std::array stgTypeFormatStrings = { "stgtst", "stgns", "stgd2ns", "stgpv" };
			const char* stgTypeFormatString = stgTypeFormatStrings[static_cast<size_t>(stageType)];

			sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/%s/%s/%s_obj.bin", objSetDirectory, stgTypeFormatString, fileName.data(), fileName.data());

			if (!FileSystem::FileExists(pathBuffer.data()))
				return false;

			objSetLoaderFunc(pathBuffer.data());

			if (stageType == StageType::STGPV)
			{
				sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/stgpv/stgpv%03d/stgpv%03d_obj.bin", objSetDirectory, stageID, stageID);
				if (FileSystem::FileExists(pathBuffer.data()))
				{
					textureObjSet = ObjSet::MakeUniqueReadParseUpload(pathBuffer.data());

					sprintf_s(pathBuffer.data(), pathBuffer.size(), "%s/stgpv/stgpv%03d/stgpv%03d_tex.bin", objSetDirectory, stageID, stageID);
					if (FileSystem::FileExists(pathBuffer.data()))
					{
						textureObjSet->TxpSet = TxpSet::MakeUniqueReadParseUpload(pathBuffer.data(), textureObjSet.get());
						renderer.RegisterTextureIDs(*textureObjSet->TxpSet);
					}
				}
			}

			return true;
		}

		template <typename T>
		bool LoadStage(SceneContext& context, StageType stageType, int stageID, int stageSubID, UniquePtr<ObjSet>& textureObjSet, T objSetLoaderFunc, D3D_Renderer3D& renderer)
		{
			LoadStageLightParamFiles(context, stageType, stageID, stageSubID);
			return LoadStageObj(context, stageType, stageID, stageSubID, textureObjSet, objSetLoaderFunc, renderer);
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

#if 1 // DEBUG:
		context.Camera.FieldOfView = 70.0f;
		context.Camera.Position = vec3(0.0f, 1.1f, 1.5f);
		cameraController.TargetCameraPitch = -11.0f;
		cameraController.TargetCameraYaw = -90.000f;
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

		if (objSet == nullptr)
		{
			LoadStage(context, StageType::STGTST, 7, 0, textureObjSet, [&](auto path) { LoadObjSet(path); }, *renderer3D);
		}
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

			if (Gui::Begin("Stage Test"))
				DrawStageTestGui();
			Gui::End();
		}
		OnWindowBegin();
	}

	void SceneRenderWindow::PostDrawGui()
	{
		return;
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
			Gui::Checkbox("Frustum Culling", &context.RenderParameters.FrustumCulling);
			Gui::Checkbox("Wireframe", &context.RenderParameters.Wireframe);
			Gui::Checkbox("Wireframe Overlay", &context.RenderParameters.WireframeOverlay);
			Gui::Checkbox("Alpha Sort", &context.RenderParameters.AlphaSort);
			Gui::Checkbox("Render Reflection", &context.RenderParameters.RenderReflection);
			Gui::Checkbox("Render Opaque", &context.RenderParameters.RenderOpaque);
			Gui::Checkbox("Render Transparent", &context.RenderParameters.RenderTransparent);
			Gui::Checkbox("Render Bloom", &context.RenderParameters.RenderBloom);
			Gui::Checkbox("Render Fog", &context.RenderParameters.RenderFog);
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

				ivec2 renderResolution = context.RenderParameters.RenderResolution;
				Gui::InputInt2("Render Resolution", glm::value_ptr(renderResolution));
				Gui::ItemContextMenu("RenderResolutionContextMenu", [&]()
				{
					Gui::Text("Set Render Resolution:");
					Gui::Separator();
					for (auto[name, factor] : namedFactors)
						if (Gui::MenuItem(name)) renderResolution = ivec2(vec2(GetRenderRegion().GetSize()) * factor);
				});

				if (renderResolution != context.RenderData.RenderTarget.GetSize())
					context.RenderParameters.RenderResolution = (clampSize(renderResolution));

				ivec2 reflectionResolution = context.RenderParameters.ReflectionRenderResolution;
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

				if (reflectionResolution != context.RenderParameters.ReflectionRenderResolution)
					context.RenderParameters.ReflectionRenderResolution = clampSize(reflectionResolution);

				if (Gui::InputScalar("Multi Sample Count", ImGuiDataType_U32, &context.RenderParameters.MultiSampleCount))
					context.RenderParameters.MultiSampleCount = std::clamp(context.RenderParameters.MultiSampleCount, 1u, 16u);
			}

			Gui::PopID();
		}

		if (Gui::CollapsingHeader("Fog", ImGuiTreeNodeFlags_None))
		{
			Gui::PushID(&context.Fog);
			Gui::SliderFloat("Density", &context.Fog.Depth.Density, 0.0f, 1.0f);
			Gui::SliderFloat("Start", &context.Fog.Depth.Start, -100.0f, 1000.0f);
			Gui::SliderFloat("End", &context.Fog.Depth.End, -100.0f, 1000.0f);
			Gui::ColorEdit3("Color", glm::value_ptr(context.Fog.Depth.Color), ImGuiColorEditFlags_Float);
			Gui::PopID();
		}

		if (Gui::CollapsingHeader("Glow", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::PushID(&context.Glow);
			Gui::SliderFloat("Exposure", &context.Glow.Exposure, 0.0f, 4.0f);
			Gui::SliderFloat("Gamma", &context.Glow.Gamma, 0.2f, 2.2f);
			Gui::SliderInt("Saturate Power", &context.Glow.SaturatePower, 1, 6);
			Gui::SliderFloat("Saturate Coefficient", &context.Glow.SaturateCoefficient, 0.0f, 1.0f);

			Gui::DragFloat3("Bloom Sigma", glm::value_ptr(context.Glow.Sigma), 0.005f, 0.0f, 3.0f);
			Gui::DragFloat3("Bloom Intensity", glm::value_ptr(context.Glow.Intensity), 0.005f, 0.0f, 2.0f);
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
				auto getMaterialName = [](Obj* obj, int index) { return (obj == nullptr || index < 0 || index > obj->Materials.size()) ? "None" : obj->Materials[index].Name.data(); };

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
					Gui::DragFloat("Transparency", &material->Transparency, 0.01f);
					Gui::ColorEdit3("Specular", glm::value_ptr(material->SpecularColor), ImGuiColorEditFlags_Float);
					Gui::DragFloat("Reflectivity", &material->Reflectivity, 0.01f);
					Gui::DragFloat("Shininess", &material->Shininess, 0.05f);
					Gui::ColorEdit3("Ambient", glm::value_ptr(material->AmbientColor), ImGuiColorEditFlags_Float);
					Gui::ColorEdit3("Emission", glm::value_ptr(material->EmissionColor), ImGuiColorEditFlags_Float);
					Gui::InputText("Material Type", material->MaterialType.data(), material->MaterialType.size(), ImGuiInputTextFlags_None);

					bool lambertShading = material->ShaderFlags.LambertShading;
					if (Gui::Checkbox("Lambert Shading", &lambertShading))
						material->ShaderFlags.LambertShading = lambertShading;

					bool phongShading = material->ShaderFlags.PhongShading;
					if (Gui::Checkbox("Phong Shading", &phongShading))
						material->ShaderFlags.PhongShading = phongShading;
				}

				Gui::PopID();
			}
		}
	}

	void SceneRenderWindow::DrawStageTestGui()
	{
		auto stageTypeGui = [&](auto& stageTypeData)
		{
			auto load = [&]()
			{
				stageTypeData.ID = std::clamp(stageTypeData.ID, stageTypeData.MinID, stageTypeData.MaxID);
				stageTypeData.SubID = std::clamp(stageTypeData.SubID, 1, 39);

				if (stageTestData.Settings.LoadLightParam)
					LoadStageLightParamFiles(context, stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID);

				if (stageTestData.Settings.LoadObj)
				{
					if (!LoadStageObj(context, stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID, textureObjSet, [&](auto path) { LoadObjSet(path); }, *renderer3D))
						objSet = nullptr;

					objectIndex = stageTestData.Settings.SelectGround ? FindGroundObj(objSet.get()) : -1;
				}
			};

			Gui::PushID(&stageTypeData);
			{
				if (Gui::Button("Reload"))
					load();

				Gui::SameLine();

				if (Gui::InputInt(stageTypeData.Name, &stageTypeData.ID, 1, 100))
				{
					stageTypeData.SubID = 1;
					load();
				}

				if (stageTypeData.Type == StageType::STGPV)
				{
					if (Gui::Button("Reload"))
						load();

					Gui::SameLine();

					if (Gui::InputInt("SUB ID", &stageTypeData.SubID))
						load();
				}
			}
			Gui::PopID();
		};

		for (auto& stageTypeData : stageTestData.TypeData)
			stageTypeGui(stageTypeData);

		Gui::Checkbox("Select Ground", &stageTestData.Settings.SelectGround);
		Gui::Checkbox("Load Light Param", &stageTestData.Settings.LoadLightParam);
		Gui::Checkbox("Load Stage", &stageTestData.Settings.LoadObj);
	}

	void SceneRenderWindow::LoadObjSet(const std::string& filePath)
	{
		if (objSet != nullptr && objSet->TxpSet != nullptr)
			renderer3D->UnRegisterTextureIDs(*objSet->TxpSet);

		objSet = nullptr;

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
	}

	void SceneRenderWindow::OnRender()
	{
		context.RenderData.OutputRenderTarget = renderTarget.get();
		context.RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

		context.Camera.UpdateMatrices();
		renderer3D->Begin(context);
		{
			if (objSet != nullptr)
			{
				if (objectIndex < 0)
				{
					for (auto& obj : *objSet)
						renderer3D->Draw(RenderCommand::ObjPos(obj, vec3(0.0f)));
				}
				else if (objectIndex < objSet->size() && !objSet->empty())
				{
					renderer3D->Draw(RenderCommand::ObjPos(*objSet->GetObjAt(objectIndex), vec3(0.0f)));
				}

				// DEBUG:
				for (auto& obj : *objSet)
				{
					if (EndsWithInsensitive(obj.Name, "_reflect"))
						renderer3D->Draw(RenderCommand::ObjPosReflect(obj, vec3(0.0f)));
				}
			}
		}
		renderer3D->End();
	}

	void SceneRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		vec2 renderRegionSize = GetRenderRegion().GetSize();
		context.Camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		context.RenderParameters.RenderResolution = renderRegionSize;
	}
}

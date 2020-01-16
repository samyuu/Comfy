#include "SceneEditor.h"
#include "Debug.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "ImGui/Extensions/TxpExtensions.h"
#include "Input/KeyCode.h"
#include <FontIcons.h>

namespace Editor
{
	using namespace Graphics;

	enum SceneEntityTag : EntityTag
	{
		NullTag = 0,
		StageTag = 'stg',
		CharacterTag = 'chr',
		ObjectTag = 'obj',
	};

	SceneEditor::SceneEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		renderer3D = MakeUnique<D3D_Renderer3D>();
		renderWindow = MakeUnique<SceneRenderWindow>(sceneGraph, context, cameraController, *renderer3D);
	}

	void SceneEditor::Initialize()
	{
		renderWindow->Initialize();

#if 1 // DEBUG:
		context.Camera.FieldOfView = 90.0f;

		context.Camera.ViewPoint = vec3(0.0f, 1.1f, 1.5f);
		context.Camera.Interest = vec3(0.0f, 1.0f, 0.0f);

		cameraController.FirstPersonData.TargetPitch = -11.0f;
		cameraController.FirstPersonData.TargetYaw = -90.000f;
#endif

		if (sceneGraph.LoadedObjSets.empty())
		{
			LoadStageObjects(StageType::STGTST, 7, 0);
			// LoadStageObjects(StageType::STGNS, 6, 0);
			// LoadStageObjects(StageType::STGPV, 260, 1);
		}
	}

	void SceneEditor::DrawGui()
	{
		Gui::GetCurrentWindow()->Hidden = true;

		RenderWindowBase::PushWindowPadding();
		if (Gui::Begin(ICON_FA_TREE "  Scene Window##SceneEditor"))
			renderWindow->DrawGui();
		Gui::End();
		RenderWindowBase::PopWindowPadding();

		if (Gui::Begin(ICON_FA_CAMERA "  Camera"))
			DrawCameraGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_FOLDER "  ObjSet Loader"))
		{
			Gui::BeginChild("ObjSetLoaderChild");
			if (objFileViewer.DrawGui())
			{
				EraseByTag(ObjectTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets));

				if (LoadRegisterObjSet(objFileViewer.GetFileToOpen(), Debug::GetTxpSetPathForObjSet(objFileViewer.GetFileToOpen()), ObjectTag))
				{
					auto& newlyAddedStageObjSet = sceneGraph.LoadedObjSets.back();
					for (auto& obj : *newlyAddedStageObjSet.ObjSet)
						sceneGraph.AddEntityFromObj(obj, ObjectTag);
				}
			}
			Gui::EndChild();
		}
		Gui::End();

		if (Gui::Begin(ICON_FA_WRENCH "  Rendering"))
			DrawRenderingGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_CLOUD_SUN "  Fog"))
			DrawFogGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_MAGIC "  Post Processing"))
			DrawPostProcessingGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_LIGHTBULB "  Light"))
			DrawLightGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_LIGHTBULB "  IBL"))
			DrawIBLGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_LIST "  Scene Entities"))
			DrawSceneGraphGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_INFO_CIRCLE "  Entity Inspector"))
			DrawEntityInspectorGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_INFO_CIRCLE "  Object Test"))
			DrawObjectTestGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_HOME "  Stage Test"))
			DrawStageTestGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_FEMALE "  Chara Test"))
			DrawCharaTestGui();
		Gui::End();
	}

	const char* SceneEditor::GetGuiName() const
	{
		return "Scene Editor";
	}

	ImGuiWindowFlags SceneEditor::GetWindowFlags() const
	{
		return BaseWindow::GetNoWindowFlags();
	}

	bool SceneEditor::LoadRegisterObjSet(std::string_view objSetPath, std::string_view txpSetPath, EntityTag tag)
	{
		if (!FileSystem::FileExists(objSetPath) || !FileSystem::FileExists(txpSetPath))
			return false;

		RefPtr<ObjSet> objSet = ObjSet::MakeUniqueReadParseUpload(objSetPath);
		objSet->Name = FileSystem::GetFileName(objSetPath, false);
		objSet->TxpSet = TxpSet::MakeUniqueReadParseUpload(txpSetPath, objSet.get());
		sceneGraph.LoadObjSet(objSet, tag);

		renderer3D->RegisterTextureIDs(*objSet->TxpSet);
		return true;
	}

	bool SceneEditor::UnLoadUnRegisterObjSet(const ObjSet* objSetToRemove)
	{
		if (objSetToRemove == nullptr)
			return false;

		renderer3D->UnRegisterTextureIDs(*objSetToRemove->TxpSet);

		sceneGraph.LoadedObjSets.erase(
			std::remove_if(sceneGraph.LoadedObjSets.begin(),
				sceneGraph.LoadedObjSets.end(),
				[&](auto& objSet) { return objSet.ObjSet.get() == objSetToRemove; }),
			sceneGraph.LoadedObjSets.end());

		return true;
	}

	bool SceneEditor::LoadStageObjects(StageType type, int id, int subID)
	{
		Debug::LoadStageLightParamFiles(context, type, id, subID);

		auto objPath = Debug::GetDebugFilePath(Debug::PathType::StageObj, type, id, subID);
		auto txpPath = Debug::GetDebugFilePath(Debug::PathType::StageTxp, type, id, subID);
		if (!LoadRegisterObjSet(objPath.data(), txpPath.data(), StageTag))
			return false;

		auto& newlyAddedStageObjSet = sceneGraph.LoadedObjSets.back();
		for (auto& obj : *newlyAddedStageObjSet.ObjSet)
		{
			auto& entity = sceneGraph.AddEntityFromObj(obj, StageTag);
			entity.IsReflection = Debug::IsReflectionObj(obj);
		}

		if (type == StageType::STGPV && subID != 0)
		{
			objPath = Debug::GetDebugFilePath(Debug::PathType::StageObj, type, id, 0);
			txpPath = Debug::GetDebugFilePath(Debug::PathType::StageTxp, type, id, 0);

			LoadRegisterObjSet(objPath.data(), txpPath.data(), StageTag);
		}

		return true;
	}

	bool SceneEditor::UnLoadStageObjects()
	{
		EraseByTag(StageTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets));
		return true;
	}

	void SceneEditor::EraseByTag(EntityTag tag, EraseFlags flags)
	{
		if (flags & EraseFlags_Entities)
		{
			sceneGraph.Entities.erase(
				std::remove_if(sceneGraph.Entities.begin(),
					sceneGraph.Entities.end(),
					[tag](auto& entity) { return entity->Tag == tag; }),
				sceneGraph.Entities.end());
		}

		if (flags & EraseFlags_ObjSets)
		{
			sceneGraph.LoadedObjSets.erase(
				std::remove_if(sceneGraph.LoadedObjSets.begin(),
					sceneGraph.LoadedObjSets.end(),
					[tag](auto& objSetResource) { return objSetResource.Tag == tag; }),
				sceneGraph.LoadedObjSets.end());
		}
	}

	void SceneEditor::DrawCameraGui()
	{
		auto& camera = context.Camera;

		Gui::PushID(&camera);
		Gui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 173.0f);
		Gui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
		Gui::DragFloat("Far Plane", &camera.FarPlane);
		Gui::DragFloat3("View Point", glm::value_ptr(camera.ViewPoint), 0.01f);
		Gui::DragFloat3("Interest", glm::value_ptr(camera.Interest), 0.01f);
		Gui::DragFloat("Smoothness", &cameraController.Settings.InterpolationSmoothness, 1.0f, 0.0f, 250.0f);

		Gui::Combo("Control Mode", reinterpret_cast<int*>(&cameraController.Mode), "None\0First Person\0Orbit\0");
		if (cameraController.Mode == CameraController3D::ControlMode::FirstPerson)
		{
			Gui::DragFloat("Camera Pitch", &cameraController.FirstPersonData.TargetPitch, 1.0f);
			Gui::DragFloat("Camera Yaw", &cameraController.FirstPersonData.TargetYaw, 1.0f);
		}
		else if (cameraController.Mode == CameraController3D::ControlMode::Orbit)
		{
			Gui::DragFloat("Orbit Distance", &cameraController.OrbitData.Distance, 0.1f, cameraController.OrbitData.MinDistance, cameraController.OrbitData.MaxDistance);
			Gui::DragFloat("Orbit X", &cameraController.OrbitData.TargetRotation.x, 1.0f);
			Gui::DragFloat("Orbit Y", &cameraController.OrbitData.TargetRotation.y, 1.0f, -89.0f, +89.0f);
		}

		Gui::Checkbox("Visualize Interest", &cameraController.Visualization.VisualizeInterest);

		if (cameraController.Visualization.VisualizeInterest && cameraController.Visualization.InterestSphereObj == nullptr)
			cameraController.Visualization.InterestSphereObj = GenerateUploadDebugSphereObj(cameraController.Visualization.InterestSphere, cameraController.Visualization.InterestSphereColor);

		Gui::PopID();
	}

	void SceneEditor::DrawRenderingGui()
	{
		auto& renderParameters = context.RenderParameters;

		Gui::PushID(&renderParameters);
		Gui::CheckboxFlags("DebugFlags_0", &renderParameters.DebugFlags, (1 << 0));
		Gui::CheckboxFlags("DebugFlags_1", &renderParameters.DebugFlags, (1 << 1));
		Gui::CheckboxFlags("DebugFlags_2", &renderParameters.DebugFlags, (1 << 2));
		Gui::ColorEdit4("Clear Color", glm::value_ptr(renderParameters.ClearColor));
		Gui::Checkbox("Clear", &renderParameters.Clear);
		Gui::Checkbox("Frustum Culling", &renderParameters.FrustumCulling);
		Gui::Checkbox("Wireframe", &renderParameters.Wireframe);
		Gui::Checkbox("Alpha Sort", &renderParameters.AlphaSort);
		Gui::Checkbox("Render Reflection", &renderParameters.RenderReflection);
		Gui::Checkbox("Render Opaque", &renderParameters.RenderOpaque);
		Gui::Checkbox("Render Transparent", &renderParameters.RenderTransparent);
		Gui::Checkbox("Render Bloom", &renderParameters.RenderBloom);
		Gui::Checkbox("Render Fog", &renderParameters.RenderFog);
		Gui::SliderInt("Anistropic Filtering", &renderParameters.AnistropicFiltering, D3D11_MIN_MAXANISOTROPY, D3D11_MAX_MAXANISOTROPY);

		if (Gui::CollapsingHeader("Resolution", ImGuiTreeNodeFlags_DefaultOpen))
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

			ivec2 renderResolution = renderParameters.RenderResolution;
			Gui::InputInt2("Render Resolution", glm::value_ptr(renderResolution));
			Gui::ItemContextMenu("RenderResolutionContextMenu", [&]()
			{
				Gui::Text("Set Render Resolution:");
				Gui::Separator();
				for (auto[name, factor] : namedFactors)
					if (Gui::MenuItem(name)) renderResolution = ivec2(vec2(renderWindow->GetRenderRegion().GetSize()) * factor);
			});

			if (renderResolution != context.RenderData.RenderTarget.GetSize())
				renderParameters.RenderResolution = (clampSize(renderResolution));

			ivec2 reflectionResolution = renderParameters.ReflectionRenderResolution;
			Gui::InputInt2("Reflection Resolution", glm::value_ptr(reflectionResolution));
			Gui::ItemContextMenu("ReflectionResolutionContextMenu", [&]()
			{
				Gui::Text("Set Reflection Resolution:");
				Gui::Separator();
				if (Gui::MenuItem("256x256")) reflectionResolution = ivec2(256, 256);
				if (Gui::MenuItem("512x512")) reflectionResolution = ivec2(512, 512);

				for (auto[name, factor] : namedFactors)
					if (Gui::MenuItem(name)) reflectionResolution = (vec2(renderWindow->GetRenderRegion().GetSize()) * factor);
			});

			if (reflectionResolution != renderParameters.ReflectionRenderResolution)
				renderParameters.ReflectionRenderResolution = clampSize(reflectionResolution);

			if (Gui::InputScalar("Multi Sample Count", ImGuiDataType_U32, &renderParameters.MultiSampleCount))
				renderParameters.MultiSampleCount = std::clamp(renderParameters.MultiSampleCount, 1u, 16u);
		}

		Gui::PopID();
	}

	void SceneEditor::DrawFogGui()
	{
		Gui::PushID(&context.Fog);

		static std::array<char, MAX_PATH> fogPathBuffer = { "dev_rom/light_param/fog_tst.txt" };
		if (Gui::InputText("Load Fog", fogPathBuffer.data(), fogPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(fogPathBuffer.data(), context.Glow);

		if (Gui::WideTreeNodeEx("Depth", ImGuiTreeNodeFlags_DefaultOpen))
		{
			Gui::SliderFloat("Density", &context.Fog.Depth.Density, 0.0f, 1.0f);
			Gui::SliderFloat("Start", &context.Fog.Depth.Start, -100.0f, 1000.0f);
			Gui::SliderFloat("End", &context.Fog.Depth.End, -100.0f, 1000.0f);
			Gui::ColorEdit3("Color", glm::value_ptr(context.Fog.Depth.Color), ImGuiColorEditFlags_Float);
			Gui::TreePop();
		}

		Gui::PopID();
	}

	void SceneEditor::DrawPostProcessingGui()
	{
		Gui::PushID(&context.Glow);

		static std::array<char, MAX_PATH> glowPathBuffer = { "dev_rom/light_param/glow_tst.txt" };
		if (Gui::InputText("Load Glow", glowPathBuffer.data(), glowPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(glowPathBuffer.data(), context.Glow);

		Gui::SliderFloat("Exposure", &context.Glow.Exposure, 0.0f, 4.0f);
		Gui::SliderFloat("Gamma", &context.Glow.Gamma, 0.2f, 2.2f);
		Gui::SliderInt("Saturate Power", &context.Glow.SaturatePower, 1, 6);
		Gui::SliderFloat("Saturate Coefficient", &context.Glow.SaturateCoefficient, 0.0f, 1.0f);

		Gui::DragFloat3("Bloom Sigma", glm::value_ptr(context.Glow.Sigma), 0.005f, 0.0f, 3.0f);
		Gui::DragFloat3("Bloom Intensity", glm::value_ptr(context.Glow.Intensity), 0.005f, 0.0f, 2.0f);
		Gui::PopID();
	}

	void SceneEditor::DrawLightGui()
	{
		auto lightGui = [](const char* name, Light& light, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
		{
			Gui::PushID(&light);
			if (Gui::WideTreeNodeEx(name, flags))
			{
				Gui::ColorEdit3("Ambient", glm::value_ptr(light.Ambient), ImGuiColorEditFlags_Float);
				Gui::ColorEdit3("Diffuse", glm::value_ptr(light.Diffuse), ImGuiColorEditFlags_Float);
				Gui::ColorEdit3("Specular", glm::value_ptr(light.Specular), ImGuiColorEditFlags_Float);
				Gui::DragFloat3("Position", glm::value_ptr(light.Position), 0.01f);
				Gui::TreePop();
			}
			Gui::PopID();
		};

		Gui::PushID(&context.Light);

		static std::array<char, MAX_PATH> lightPathBuffer = { "dev_rom/light_param/light_tst.txt" };
		if (Gui::InputText("Load Light", lightPathBuffer.data(), lightPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(lightPathBuffer.data(), context.Light);

		lightGui("Character Light", context.Light.Character, ImGuiTreeNodeFlags_DefaultOpen);
		lightGui("Stage Light", context.Light.Stage, ImGuiTreeNodeFlags_DefaultOpen);

		Gui::PopID();
	}

	void SceneEditor::DrawIBLGui()
	{
		auto iblLightDataGui = [](const char* name, LightData& lightData, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
		{
			Gui::PushID(&lightData);
			if (Gui::WideTreeNodeEx(name, flags))
			{
				Gui::ColorEdit3("Light Color", glm::value_ptr(lightData.LightColor), ImGuiColorEditFlags_Float);
				// Gui::DragFloat3("Light Direction", glm::value_ptr(lightData.LightDirection), 0.01f);

				constexpr float cubeMapSize = 60.0f;
				if (lightData.LightMap.CubeMap != nullptr)
					Gui::ImageButton(*lightData.LightMap.CubeMap, vec2(cubeMapSize, cubeMapSize * (3.0f / 4.0f)));

				Gui::TreePop();
			}
			Gui::PopID();
		};

		Gui::PushID(&context.IBL);

		static std::array<char, MAX_PATH> iblPathBuffer = { "dev_rom/ibl/tst.ibl" };
		if (Gui::InputText("Load IBL", iblPathBuffer.data(), iblPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(iblPathBuffer.data(), context.IBL);

		iblLightDataGui("Stage", context.IBL.Stage, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Character", context.IBL.Character, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Sun", context.IBL.Sun, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Reflect", context.IBL.Reflect, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Shadow", context.IBL.Shadow, ImGuiTreeNodeFlags_DefaultOpen);

		Gui::PopID();
	}

	void SceneEditor::DrawSceneGraphGui()
	{
		Gui::BeginChild("SceneGraphEntiriesListChild");

		auto setCamera = [&](auto& entity, const Sphere& boundingSphere)
		{
			context.Camera.Interest = context.Camera.ViewPoint = boundingSphere.Center + entity->Transform.Translation;
			cameraController.OrbitData.Distance = boundingSphere.Radius;
		};

		for (auto& entity : sceneGraph.Entities)
		{
			if (!entity->IsVisible) Gui::PushStyleColor(ImGuiCol_Text, Gui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			const bool objectNodeOpen = Gui::WideTreeNodeEx(entity->Name.c_str(), ImGuiTreeNodeFlags_None);
			if (!entity->IsVisible) Gui::PopStyleColor();

			if (Gui::IsItemClicked(1))
				setCamera(entity, entity->Obj->BoundingSphere);

			if (objectNodeOpen)
			{
				for (auto& mesh : entity->Obj->Meshes)
				{
					Gui::WideTreeNodeEx(mesh.Name.data(), ImGuiTreeNodeFlags_NoTreePushOnOpen | ImGuiTreeNodeFlags_Leaf);

					if (Gui::IsItemClicked(1))
						setCamera(entity, mesh.BoundingSphere);
				}

				Gui::TreePop();
			}
		}

		Gui::EndChild();
	}

	void SceneEditor::DrawEntityInspectorGui()
	{
		if (Gui::InputInt("Entity Index", &inspector.EntityIndex, 1, 10))
			inspector.EntityIndex = std::clamp(inspector.EntityIndex, -1, static_cast<int>(sceneGraph.Entities.size()) - 1);

		auto getName = [&](int index) { return (index < 0 || index >= sceneGraph.Entities.size()) ? "None" : sceneGraph.Entities[index]->Name.c_str(); };

		if (Gui::BeginCombo("Obj", getName(inspector.EntityIndex), ImGuiComboFlags_HeightLarge))
		{
			for (int objIndex = 0; objIndex < static_cast<int>(sceneGraph.Entities.size()); objIndex++)
			{
				if (Gui::Selectable(getName(objIndex), (objIndex == inspector.EntityIndex)))
					inspector.EntityIndex = objIndex;

				if (objIndex == inspector.EntityIndex)
					Gui::SetItemDefaultFocus();
			}

			Gui::EndCombo();
		}

		if (inspector.EntityIndex < 0 || inspector.EntityIndex >= sceneGraph.Entities.size())
			return;

		auto& entity = sceneGraph.Entities[inspector.EntityIndex];
		Gui::PushID(&entity);

		Gui::InputScalar("Tag", ImGuiDataType_S64, &entity->Tag, nullptr, nullptr, nullptr, ImGuiInputTextFlags_ReadOnly);
		Gui::Checkbox("Visible", &entity->IsVisible);

		// TODO: Gui::ComfyFloat3TextWidget();
		Gui::DragFloat3("Translation", glm::value_ptr(entity->Transform.Translation), 0.1f);
		Gui::DragFloat3("Rotation", glm::value_ptr(entity->Transform.Rotation), 0.1f);
		Gui::DragFloat3("Scale", glm::value_ptr(entity->Transform.Scale), 0.01f);

		Gui::PopID();
	}

	void SceneEditor::DrawObjectTestGui()
	{
		if (objTestData.ObjSetIndex < 0 || objTestData.ObjSetIndex >= sceneGraph.LoadedObjSets.size())
		{
			objTestData.ObjSetIndex = 0;
			return;
		}

		if (Gui::InputInt("ObjSet Index", &objTestData.ObjSetIndex, 1, 10))
			objTestData.ObjSetIndex = std::clamp(objTestData.ObjSetIndex, 0, static_cast<int>(sceneGraph.LoadedObjSets.size()) - 1);

		auto getObjSetName = [&](int index) { return (index < 0 || index >= sceneGraph.LoadedObjSets.size()) ? "None" : sceneGraph.LoadedObjSets[index].ObjSet->Name.c_str(); };

		// TODO: Refactor combo box vector + index into helper function
		if (Gui::BeginCombo("ObjSet", getObjSetName(objTestData.ObjSetIndex), ImGuiComboFlags_HeightLarge))
		{
			for (int setIndex = 0; setIndex < static_cast<int>(sceneGraph.LoadedObjSets.size()); setIndex++)
			{
				if (Gui::Selectable(getObjSetName(setIndex), (setIndex == objTestData.ObjSetIndex)))
					objTestData.ObjSetIndex = setIndex;

				if (setIndex == objTestData.ObjSetIndex)
					Gui::SetItemDefaultFocus();
			}

			Gui::ComfyEndCombo();
		}

		if (objTestData.ObjSetIndex < 0 || objTestData.ObjSetIndex >= sceneGraph.LoadedObjSets.size())
			return;

		ObjSet& objSet = *sceneGraph.LoadedObjSets[objTestData.ObjSetIndex].ObjSet;

		auto getObjName = [&](int index) { return (index < 0 || index >= objSet.size()) ? "None" : objSet.GetObjAt(index)->Name.c_str(); };

		if (Gui::InputInt("Object Index", &objTestData.ObjIndex, 1, 10))
			objTestData.ObjIndex = std::clamp(objTestData.ObjIndex, 0, static_cast<int>(objSet.size()) - 1);

		if (Gui::BeginCombo("Object", getObjName(objTestData.ObjIndex), ImGuiComboFlags_HeightLarge))
		{
			for (int objIndex = 0; objIndex < static_cast<int>(objSet.size()); objIndex++)
			{
				if (Gui::Selectable(getObjName(objIndex), (objIndex == objTestData.ObjIndex)))
					objTestData.ObjIndex = objIndex;

				if (objIndex == objTestData.ObjIndex)
					Gui::SetItemDefaultFocus();
			}

			Gui::ComfyEndCombo();
		}

		Obj* obj = (objTestData.ObjIndex >= 0 && objTestData.ObjIndex < objSet.size()) ? objSet.GetObjAt(objTestData.ObjIndex) : nullptr;

		if (Gui::CollapsingHeader("Material Test"))
		{
			auto getMaterialName = [](Obj* obj, int index) { return (obj == nullptr || index < 0 || index >= obj->Materials.size()) ? "None" : obj->Materials[index].Name.data(); };

			if (Gui::InputInt("Material Index", &objTestData.MaterialIndex, 1, 10))
				objTestData.MaterialIndex = std::clamp(objTestData.MaterialIndex, 0, (obj != nullptr) ? static_cast<int>(obj->Materials.size()) - 1 : 0);

			if (Gui::BeginCombo("Material", getMaterialName(obj, objTestData.MaterialIndex), ImGuiComboFlags_HeightLarge))
			{
				if (obj != nullptr)
				{
					for (int matIndex = 0; matIndex < static_cast<int>(obj->Materials.size()); matIndex++)
					{
						if (Gui::Selectable(getMaterialName(obj, matIndex), (matIndex == objTestData.MaterialIndex)))
							objTestData.MaterialIndex = matIndex;

						if (matIndex == objTestData.MaterialIndex)
							Gui::SetItemDefaultFocus();
					}
				}

				Gui::EndCombo();
			}

			Material* material = (obj != nullptr && objTestData.MaterialIndex >= 0 && objTestData.MaterialIndex < obj->Materials.size()) ?
				&obj->Materials[objTestData.MaterialIndex] : nullptr;

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

				int materialIndex = 0;
				material->IterateTextures([&](MaterialTexture* materialTexture)
				{
					std::array names = { "Diffuse", "Ambient", "Normal", "Specular", "ToonCurve", "Reflection", "Lucency", "" };
					auto textureTypeName = names[materialIndex++];

					if (auto txp = renderer3D->GetTxpFromTextureID(materialTexture->TextureID); txp != nullptr)
					{
						Gui::ImageObjTxp(txp, vec2(120.0f));

						if (Gui::IsItemHovered())
							Gui::SetTooltip(textureTypeName);
					}
				});
			}
		}

		if (Gui::CollapsingHeader("Mesh Test"))
		{
			auto getMeshName = [](Obj* obj, int index) { return (obj == nullptr || index < 0 || index >= obj->Meshes.size()) ? "None" : obj->Meshes[index].Name.data(); };

			if (Gui::InputInt("Mesh Index", &objTestData.MeshIndex, 1, 10))
				objTestData.MeshIndex = std::clamp(objTestData.MeshIndex, 0, (obj != nullptr) ? static_cast<int>(obj->Meshes.size()) - 1 : 0);

			if (Gui::BeginCombo("Mesh", getMeshName(obj, objTestData.MeshIndex), ImGuiComboFlags_HeightLarge))
			{
				if (obj != nullptr)
				{
					for (int meshIndex = 0; meshIndex < static_cast<int>(obj->Meshes.size()); meshIndex++)
					{
						if (Gui::Selectable(getMeshName(obj, meshIndex), (meshIndex == objTestData.MeshIndex)))
							objTestData.MeshIndex = meshIndex;

						if (meshIndex == objTestData.MeshIndex)
							Gui::SetItemDefaultFocus();
					}
				}

				Gui::EndCombo();
			}

			Mesh* mesh = (obj != nullptr && objTestData.MeshIndex >= 0 && objTestData.MeshIndex < obj->Meshes.size()) ?
				&obj->Meshes[objTestData.MeshIndex] : nullptr;

			if (mesh != nullptr)
			{
				bool faceCamera = mesh->Flags.FaceCamera;
				if (Gui::Checkbox("Face Camera", &faceCamera))
					mesh->Flags.FaceCamera = faceCamera;

				// TODO:
				Gui::Checkbox("Show Bounding Sphere", &mesh->Debug.RenderBoundingSphere);
			}
		}
	}

	void SceneEditor::DrawStageTestGui()
	{
		auto stageTypeGui = [&](auto& stageTypeData)
		{
			auto load = [&]()
			{
				stageTypeData.ID = std::clamp(stageTypeData.ID, stageTypeData.MinID, stageTypeData.MaxID);
				stageTypeData.SubID = std::clamp(stageTypeData.SubID, 1, 39);

				if (stageTestData.Settings.LoadLightParam)
					Debug::LoadStageLightParamFiles(context, stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID);

				if (stageTestData.Settings.LoadObj)
				{
					UnLoadStageObjects();
					LoadStageObjects(stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID);
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

		Gui::Checkbox("Load Light Param", &stageTestData.Settings.LoadLightParam);
		Gui::Checkbox("Load Stage", &stageTestData.Settings.LoadObj);
	}

	void SceneEditor::DrawCharaTestGui()
	{
		auto unloadCharaItems = [&] { EraseByTag(CharacterTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets)); };

		auto loadCharaItems = [&]()
		{
			auto loadPart = [&](int id, int exclusiveObjIndex = -1)
			{
				auto objSetPath = Debug::GetDebugFilePath(Debug::PathType::CharaItemObj, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());
				auto txpSetPath = Debug::GetDebugFilePath(Debug::PathType::CharaItemTxp, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());

				if (LoadRegisterObjSet(objSetPath.data(), txpSetPath.data(), CharacterTag))
				{
					auto& loadedResource = sceneGraph.LoadedObjSets.back();

					if (exclusiveObjIndex < 0 || exclusiveObjIndex >= loadedResource.ObjSet->size())
					{
						for (auto& obj : *loadedResource.ObjSet)
							sceneGraph.AddEntityFromObj(obj, CharacterTag);
					}
					else
					{
						sceneGraph.AddEntityFromObj(loadedResource.ObjSet->at(exclusiveObjIndex), CharacterTag);
					}
				}
			};

			unloadCharaItems();
			loadPart(charaTestData.IDs.Face, charaTestData.IDs.FaceIndex);
			loadPart(charaTestData.IDs.Overhead);
			loadPart(charaTestData.IDs.Hair);
			loadPart(charaTestData.IDs.Outer);
			loadPart(charaTestData.IDs.Hands);
		};

		Gui::PushID(&charaTestData);
		Gui::InputText("Character", charaTestData.IDs.Character.data(), charaTestData.IDs.Character.size());

		if (Gui::InputInt("Face", &charaTestData.IDs.FaceIndex))
			loadCharaItems();

		if (Gui::InputInt("Overhead", &charaTestData.IDs.Overhead))
			loadCharaItems();

		if (Gui::InputInt("Hair", &charaTestData.IDs.Hair))
			loadCharaItems();

		if (Gui::InputInt("Outer", &charaTestData.IDs.Outer))
			loadCharaItems();

		if (Gui::InputInt("Hands", &charaTestData.IDs.Hands))
			loadCharaItems();

		bool posChanged = Gui::DragFloat3("Position", glm::value_ptr(charaTestData.Transform.Translation), 0.1f);
		bool rotChanged = Gui::DragFloat3("Rotation", glm::value_ptr(charaTestData.Transform.Rotation), 0.1f);

		if (posChanged | rotChanged)
		{
			for (auto& entity : sceneGraph.Entities)
			{
				if (entity->Tag == CharacterTag)
					entity->Transform = charaTestData.Transform;
			}
		}

		if (Gui::Button("Reload"))
			loadCharaItems();
		Gui::SameLine();
		if (Gui::Button("Unload"))
			unloadCharaItems();

		Gui::PopID();
	}
}

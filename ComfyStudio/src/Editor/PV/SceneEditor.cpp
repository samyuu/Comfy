#include "SceneEditor.h"
#include "Debug.h"
#include "Graphics/Auth3D/A3D/A3D.h"
#include "Graphics/Auth3D/A3D/A3DMgr.h"
#include "Graphics/Auth3D/Misc/DebugObj.h"
#include "IO/Archive/FArc.h"
#include "IO/Shell.h"
#include "ImGui/Extensions/TexExtensions.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "Misc/ImageHelper.h"
#include "Time/TimeUtilities.h"
#include "Input/Input.h"
#include "System/ComfyData.h"
#include <FontIcons.h>

namespace Comfy::Studio::Editor
{
	using namespace Graphics;

	static constexpr const char* ScreenshotDirectoy = "dev_ram/ss";

	enum SceneEntityTag : EntityTag
	{
		NullTag = 0,
		StageTag = 'stg',
		CharacterTag = 'chr',
		ObjectTag = 'obj',
	};

	SceneEditor::SceneEditor(Application& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		auto texGetter = [&](const Cached_TexID* texID) { return sceneGraph.TexIDMap.Find(texID); };
		renderer3D = std::make_unique<Render::Renderer3D>(texGetter);
		renderWindow = std::make_unique<SceneRenderWindow>(sceneGraph, camera, *renderer3D, scene, cameraController);
	}

	void SceneEditor::OnFirstFrame()
	{
		camera.FieldOfView = 90.0f;
		camera.ViewPoint = vec3(0.0f, 1.1f, 1.5f);
		camera.Interest = vec3(0.0f, 1.0f, 0.0f);

		cameraController.FirstPersonData.TargetPitch = -11.0f;
		cameraController.FirstPersonData.TargetYaw = -90.000f;

		if (sceneGraph.LoadedObjSets.empty())
		{
			LoadStageObjects(StageType::STGTST, 7, 0);
			SetStageVisibility(StageVisibilityType::GroundSky);
		}
	}

	const char* SceneEditor::GetName() const
	{
		return "Scene Editor";
	}

	ImGuiWindowFlags SceneEditor::GetFlags() const
	{
		return BaseWindow::NoWindowFlags;
	}

	void SceneEditor::Gui()
	{
		Gui::GetCurrentWindow()->Hidden = true;

		renderWindow->BeginEndGui(ICON_FA_TREE "  Scene Window##SceneEditor");

		if (Gui::Begin(ICON_FA_CAMERA "  Camera"))
			DrawCameraGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_FOLDER "  ObjSet Loader"))
			DrawObjSetLoaderGui();
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

		if (Gui::Begin(ICON_FA_SYNC_ALT "  A3D Test"))
			DrawA3DTestGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_PROJECT_DIAGRAM "  External Process"))
			DrawExternalProcessTestGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_BUG "  Debug Test"))
			DrawDebugTestGui();
		Gui::End();
	}

	bool SceneEditor::LoadRegisterObjSet(std::string_view objSetPath, std::string_view texSetPath, EntityTag tag)
	{
		if (!IO::File::Exists(objSetPath) || !IO::File::Exists(texSetPath))
			return false;

		if (objSetPath == texSetPath)
			return false;

		std::shared_ptr<ObjSet> objSet = ObjSet::MakeUniqueReadParseUpload(objSetPath);
		objSet->Name = IO::Path::GetFileName(objSetPath, false);
		objSet->TexSet = TexSet::MakeUniqueReadParseUpload(texSetPath, objSet.get());
		sceneGraph.LoadObjSet(objSet, tag);
		sceneGraph.RegisterTextures(objSet->TexSet.get());

		if (sceneGraph.TexDB == nullptr)
		{
			constexpr std::string_view texDBPath = "dev_rom/db/tex_db.bin";
			sceneGraph.TexDB = IO::File::Load<Database::TexDB>(texDBPath);
		}

		if (sceneGraph.TexDB != nullptr && objSet->TexSet != nullptr)
		{
			for (auto& tex : objSet->TexSet->Textures)
			{
				// TODO: Linear search yikesydoodles
				if (const auto matchingEntry = FindIfOrNull(sceneGraph.TexDB->Entries, [&](const auto& e) { return e.ID == tex->ID; }); matchingEntry != nullptr)
					tex->Name.emplace(matchingEntry->Name);
			}
		}

		return true;
	}

	bool SceneEditor::UnLoadUnRegisterObjSet(const ObjSet* objSetToRemove)
	{
		if (objSetToRemove == nullptr)
			return false;

		if (objSetToRemove->TexSet != nullptr)
		{
			sceneGraph.TexIDMap.RemoveIf([&](auto& pair)
			{
				return std::any_of(objSetToRemove->TexSet->Textures.begin(), objSetToRemove->TexSet->Textures.end(), [&pair](auto& tex) { return tex->ID == pair.ID; });
			});
		}

		sceneGraph.LoadedObjSets.erase(
			std::remove_if(sceneGraph.LoadedObjSets.begin(),
				sceneGraph.LoadedObjSets.end(),
				[&](auto& objSet) { return objSet.ObjSet.get() == objSetToRemove; }),
			sceneGraph.LoadedObjSets.end());

		return true;
	}

	bool SceneEditor::LoadStageObjects(StageType type, int id, int subID, bool loadLightParam)
	{
		if (loadLightParam)
			Debug::LoadStageLightParamFiles(scene, type, id, subID);

		auto objPath = Debug::GetDebugFilePath(Debug::PathType::StageObj, type, id, subID);
		auto texPath = Debug::GetDebugFilePath(Debug::PathType::StageTex, type, id, subID);
		if (!LoadRegisterObjSet(objPath.data(), texPath.data(), StageTag))
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
			texPath = Debug::GetDebugFilePath(Debug::PathType::StageTex, type, id, 0);

			LoadRegisterObjSet(objPath.data(), texPath.data(), StageTag);
		}

		return true;
	}

	bool SceneEditor::UnLoadStageObjects()
	{
		EraseByTag(StageTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets));
		return true;
	}

	void SceneEditor::SetStageVisibility(StageVisibilityType visibility)
	{
		std::for_each(sceneGraph.Entities.begin(), sceneGraph.Entities.end(), [visibility](auto& e)
		{
			if (e->Tag != StageTag)
				return;

			if (visibility == StageVisibilityType::None)
				e->IsVisible = false;
			else if (visibility == StageVisibilityType::All)
				e->IsVisible = true;
			else if (visibility == StageVisibilityType::GroundSky)
				e->IsVisible = (e->Obj != nullptr) && (Debug::IsGroundOrSkyObj(*e->Obj) || Debug::IsReflectionObj(*e->Obj));
		});
	}

	void SceneEditor::EraseByTag(EntityTag tag, EraseFlags flags)
	{
		if (flags & EraseFlags_Entities)
		{
			auto checkTag = [tag](auto& entity) { return entity->Tag == tag; };

			sceneGraph.Entities.erase(
				std::remove_if(sceneGraph.Entities.begin(), sceneGraph.Entities.end(),
					checkTag),
				sceneGraph.Entities.end());
		}

		if (flags & EraseFlags_ObjSets)
		{
			auto checkTagUnregisterTex = [&](ObjSetResource& objSetResource)
			{
				if (objSetResource.Tag == tag)
				{
					if (objSetResource.ObjSet->TexSet != nullptr)
					{
						sceneGraph.TexIDMap.RemoveIf([&](auto& pair)
						{
							return std::any_of(objSetResource.ObjSet->TexSet->Textures.begin(), objSetResource.ObjSet->TexSet->Textures.end(), [&pair](auto& tex) { return tex->ID == pair.ID; });
						});
					}
					return true;
				}
				return false;
			};

			sceneGraph.LoadedObjSets.erase(
				std::remove_if(sceneGraph.LoadedObjSets.begin(), sceneGraph.LoadedObjSets.end(),
					checkTagUnregisterTex),
				sceneGraph.LoadedObjSets.end());
		}
	}

	void SceneEditor::DrawCameraGui()
	{
		GuiPropertyRAII::PropertyValueColumns columns;
		GuiPropertyRAII::ID id(&camera);

		GuiProperty::Input("Field Of View", camera.FieldOfView, 1.0f, vec2(1.0f, 173.0f));
		GuiProperty::Input("Near Plane", camera.NearPlane, 0.001f, vec2(0.001f, 1.0f));
		GuiProperty::Input("Far Plane", camera.FarPlane, 10.0f);
		GuiProperty::Input("View Point", camera.ViewPoint, 0.01f);
		GuiProperty::Input("Interest", camera.Interest, 0.01f);
		GuiProperty::Combo("Control Mode", cameraController.Mode, CameraController3D::ControlModeNames, ImGuiComboFlags_None);

		Gui::ItemContextMenu("ControlModeContextMenu", [&]()
		{
			Gui::TextUnformatted("Camera Presets:");
			Gui::Separator();
			if (Gui::MenuItem("Orbit Default"))
			{
				cameraController.Mode = CameraController3D::ControlMode::Orbit;
				cameraController.OrbitData.Distance = 5.0f;
				cameraController.OrbitData.TargetRotation = vec3(0.0f);
				camera.FieldOfView = 90.0f;
			}
			if (Gui::MenuItem("None Default"))
			{
				cameraController.Mode = CameraController3D::ControlMode::None;
				camera.ViewPoint = vec3(0.0f, 0.88f, 4.3f);
				camera.Interest = vec3(0.0f, 1.0f, 0.0f);
				camera.FieldOfView = 32.26734161f;
			}
		});

		if (cameraController.Mode == CameraController3D::ControlMode::FirstPerson)
		{
			GuiProperty::Input("Camera Pitch", cameraController.FirstPersonData.TargetPitch, 1.0f);
			GuiProperty::Input("Camera Yaw", cameraController.FirstPersonData.TargetYaw, 1.0f);
		}
		else if (cameraController.Mode == CameraController3D::ControlMode::Orbit)
		{
			GuiProperty::Input("Orbit Distance", cameraController.OrbitData.Distance, 0.1f, vec2(cameraController.OrbitData.MinDistance, cameraController.OrbitData.MaxDistance));
			GuiProperty::Input("Orbit X", cameraController.OrbitData.TargetRotation.x, 1.0f);
			GuiProperty::Input("Orbit Y", cameraController.OrbitData.TargetRotation.y, 1.0f, vec2(-89.0f, +89.0f));
		}

		GuiProperty::Checkbox("Visualize Interest", cameraController.Visualization.VisualizeInterest);

		if (cameraController.Visualization.VisualizeInterest && cameraController.Visualization.InterestSphereObj == nullptr)
			cameraController.Visualization.InterestSphereObj = GenerateDebugSphereObj(cameraController.Visualization.InterestSphere, cameraController.Visualization.InterestSphereColor);
	}

	void SceneEditor::DrawObjSetLoaderGui()
	{
		{
			GuiPropertyRAII::PropertyValueColumns columns;
			GuiPropertyRAII::ID id(&objSetTransform);

			bool transformChanged = false;
			transformChanged |= GuiProperty::Input("Translation", objSetTransform.Translation, 0.1f);
			transformChanged |= GuiProperty::Input("Scale", objSetTransform.Scale, 0.05f);
			transformChanged |= GuiProperty::Input("Rotation", objSetTransform.Rotation, 1.0f);

			if (transformChanged)
			{
				for (auto& entity : sceneGraph.Entities)
					if (entity->Tag == ObjectTag) entity->Transform = objSetTransform;
			}
		}

		Gui::BeginChild("ObjSetLoaderChild");
		if (objSetFileViewer.DrawGui())
		{
			EraseByTag(ObjectTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets));

			if (LoadRegisterObjSet(objSetFileViewer.GetFileToOpen(), Debug::GetTexSetPathForObjSet(objSetFileViewer.GetFileToOpen()), ObjectTag))
			{
				auto& newlyAddedStageObjSet = sceneGraph.LoadedObjSets.back();
				for (auto& obj : *newlyAddedStageObjSet.ObjSet)
					sceneGraph.AddEntityFromObj(obj, ObjectTag);
			}
		}
		Gui::EndChild();
	}

	void SceneEditor::DrawRenderingGui()
	{
		if (renderWindow->GetRenderTarget() == nullptr)
			return;

		auto& renderParam = renderWindow->GetRenderTarget()->Param;
		GuiPropertyRAII::ID id(&renderParam);

		TakeScreenshotGui();

		GuiProperty::TreeNode("Render Parameters", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, [&]
		{
			auto resolutionGui = [&](const char* label, const char* contextMenu, ivec2& inOutResolution)
			{
				auto clampValidTextureSize = [](ivec2 size) { return glm::clamp(size, Render::RenderTargetMinSize, Render::RenderTargetMaxSize); };

				if (GuiProperty::Input(label, inOutResolution))
					inOutResolution = clampValidTextureSize(inOutResolution);

				Gui::ItemContextMenu(contextMenu, [&]
				{
					static constexpr std::array fixedResolutions = { ivec2(256, 256), ivec2(512, 512), ivec2(1024, 1024), ivec2(2048, 2048), ivec2(4096, 4096), ivec2(8192, 8192) };
					static constexpr std::array scaleFactors = { 0.25f, 0.50f, 0.75f, 1.00f, 1.50f, 2.00f, 4.00f, 8.00f };

					char nameBuffer[32];

					Gui::Text("Set Resolution:");
					Gui::Separator();
					for (const ivec2 resolution : fixedResolutions)
					{
						sprintf_s(nameBuffer, "%dx%d", resolution.x, resolution.y);
						if (Gui::MenuItem(nameBuffer))
							inOutResolution = clampValidTextureSize(resolution);
					}
					Gui::Separator();
					for (const float factor : scaleFactors)
					{
						sprintf_s(nameBuffer, "Render Region x%.2f", factor);
						if (Gui::MenuItem(nameBuffer))
							inOutResolution = clampValidTextureSize(ivec2(vec2(renderWindow->GetRenderRegion().GetSize()) * factor));
					}
				});
			};

			GuiPropertyRAII::PropertyValueColumns columns;
			GuiProperty::TreeNode("Debug", [&]
			{
				GuiProperty::CheckboxFlags("DebugFlags_0", renderParam.DebugFlags, (1 << 0));
				GuiProperty::CheckboxFlags("DebugFlags_1", renderParam.DebugFlags, (1 << 1));
				GuiProperty::CheckboxFlags("ShaderDebugFlags_0", renderParam.ShaderDebugFlags, (1 << 0));
				GuiProperty::CheckboxFlags("ShaderDebugFlags_1", renderParam.ShaderDebugFlags, (1 << 1));
				GuiProperty::ColorEditHDR("ShaderDebugValue", renderParam.ShaderDebugValue);

				GuiProperty::Checkbox("Visualize Occlusion Query", renderParam.DebugVisualizeOcclusionQuery);
				GuiProperty::Checkbox("Occlusion Query Optimization", renderParam.LastFrameOcclusionQueryOptimization);
			});

			GuiProperty::TreeNode("General", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				resolutionGui("Main Scene", "RenderResolutionContextMenu", renderParam.RenderResolution);
				resolutionGui("Reflection", "ReflectionResolutionContextMenu", renderParam.ReflectionRenderResolution);

				if (GuiProperty::Input("Multi Sample Count", renderParam.MultiSampleCount))
					renderParam.MultiSampleCount = std::clamp(renderParam.MultiSampleCount, 1u, 16u);

				if (GuiProperty::Input("Anistropic Filtering", renderParam.AnistropicFiltering))
					renderParam.AnistropicFiltering = std::clamp(renderParam.AnistropicFiltering, Render::AnistropicFilteringMin, Render::AnistropicFilteringMax);

				GuiProperty::ColorEdit("Clear Color", renderParam.ClearColor);
				GuiProperty::Checkbox("Clear", renderParam.Clear);
				GuiProperty::Checkbox("Preserve Alpha", renderParam.ToneMapPreserveAlpha);

				GuiProperty::Checkbox("Frustum Culling", renderParam.FrustumCulling);
				GuiProperty::Checkbox("Alpha Sort", renderParam.AlphaSort);
				GuiProperty::Checkbox("Wireframe", renderParam.Wireframe);
			});

			GuiProperty::TreeNode("Shadow Mapping", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Shadow Mapping", renderParam.ShadowMapping);
				GuiProperty::Checkbox("Self Shadowing", renderParam.SelfShadowing);

				resolutionGui("Resolution", "ShadowMapResolutionContextMenu", renderParam.ShadowMapResolution);

				if (GuiProperty::Input("Blur Passes", renderParam.ShadowBlurPasses))
					renderParam.ShadowBlurPasses = std::clamp(renderParam.ShadowBlurPasses, 0u, 10u);
			});

			GuiProperty::TreeNode("Render Passes", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Reflection", renderParam.RenderReflection);
				GuiProperty::Checkbox("Subsurface Scattering", renderParam.RenderSubsurfaceScattering);
				GuiProperty::Checkbox("Opaque", renderParam.RenderOpaque);
				GuiProperty::Checkbox("Transparent", renderParam.RenderTransparent);
				GuiProperty::Checkbox("Bloom", renderParam.RenderBloom);
				GuiProperty::Checkbox("Lens Flare", renderParam.RenderLensFlare);
				GuiProperty::Checkbox("Auto Exposure", renderParam.AutoExposure);
			});

			GuiProperty::TreeNode("Shader", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Vertex Coloring", renderParam.VertexColoring);
				GuiProperty::Checkbox("Diffuse Mapping", renderParam.DiffuseMapping);
				GuiProperty::Checkbox("Ambient Occlusion Mapping", renderParam.AmbientOcclusionMapping);
				GuiProperty::Checkbox("Normal Mapping", renderParam.NormalMapping);
				GuiProperty::Checkbox("Specular Mapping", renderParam.SpecularMapping);
				GuiProperty::Checkbox("Transparency Mapping", renderParam.TransparencyMapping);
				GuiProperty::Checkbox("Environment Mapping", renderParam.EnvironmentMapping);
				GuiProperty::Checkbox("Translucency Mapping", renderParam.TranslucencyMapping);
			});

			GuiProperty::TreeNode("Other", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Punch Through", renderParam.RenderPunchThrough);
				GuiProperty::Checkbox("Render Fog", renderParam.RenderFog);
				GuiProperty::Checkbox("Object Billboarding", renderParam.ObjectBillboarding);
				GuiProperty::Checkbox("Object Morphing", renderParam.ObjectMorphing);
				GuiProperty::Checkbox("Object Skinning", renderParam.ObjectSkinning);
			});

		});

		GuiProperty::TreeNode("Render Targets", ImGuiTreeNodeFlags_NoTreePushOnOpen, [&]
		{
			if (renderWindow->GetRenderTarget() == nullptr)
				return;

			const auto subTargets = renderWindow->GetRenderTarget()->GetSubTargets();
			for (int index = 0; index < static_cast<int>(subTargets.Count); index++)
			{
				const auto& subTarget = subTargets.Targets[index];

				const u32 openMask = (1 << index);
				Gui::Selectable(subTarget.Name);

				if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(0))
					openRenderTargetsFlags ^= openMask;

				const float aspectRatio = (static_cast<float>(subTarget.Size.y) / static_cast<float>(subTarget.Size.x));
				if (openRenderTargetsFlags & openMask)
				{
					constexpr float desiredWidth = 512.0f;

					bool open = true;
					if (Gui::Begin(subTarget.Name, &open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoDocking))
						Gui::Image(subTarget.TextureID, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::End();
					if (!open)
						openRenderTargetsFlags &= ~openMask;
				}
				else if (Gui::IsItemHovered())
				{
					constexpr float desiredWidth = 256.0f;

					Gui::BeginTooltip();
					Gui::Image(subTarget.TextureID, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::EndTooltip();
				}
			}
		});
	}

	void SceneEditor::DrawFogGui()
	{
		GuiPropertyRAII::ID id(&scene.Fog);
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load Fog File", fogPathBuffer.data(), fogPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(fogPathBuffer.data(), scene.Glow);

		GuiProperty::TreeNode("Depth", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			GuiProperty::Input("Density", scene.Fog.Depth.Density, 0.005f, vec2(0.0f, 1.0f));
			GuiProperty::Input("Start", scene.Fog.Depth.Start, 0.1f, vec2(-100.0f, 1000.0f));
			GuiProperty::Input("End", scene.Fog.Depth.End, 0.1f, vec2(-100.0f, 1000.0f));
			GuiProperty::ColorEditHDR("Color", scene.Fog.Depth.Color, ImGuiColorEditFlags_Float);
		});
	}

	void SceneEditor::DrawPostProcessingGui()
	{
		GuiPropertyRAII::ID id(&scene.Glow);
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load Glow File", glowPathBuffer.data(), glowPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(glowPathBuffer.data(), scene.Glow);

		GuiProperty::Input("Exposure", scene.Glow.Exposure, 0.005f, vec2(0.0f, 4.0f));
		GuiProperty::Input("Gamma", scene.Glow.Gamma, 0.005f, vec2(0.2f, 2.2f));
		GuiProperty::Input("Saturate Power", scene.Glow.SaturatePower, 0.1f, ivec2(1, 6));
		GuiProperty::Input("Saturate Coefficient", scene.Glow.SaturateCoefficient, 0.005f, vec2(0.0f, 1.0f));
		GuiProperty::Input("Bloom Sigma", scene.Glow.Sigma, 0.005f, vec2(0.0f, 3.0f));
		GuiProperty::Input("Bloom Intensity", scene.Glow.Intensity, 0.005f, vec2(0.0f, 2.0f));
		GuiProperty::Checkbox("Auto Exposure", scene.Glow.AutoExposure);
	}

	void SceneEditor::DrawLightGui()
	{
		auto lightGui = [](std::string_view name, Light& light, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
		{
			GuiPropertyRAII::ID id(&light);

			GuiProperty::TreeNode(name, flags, [&]
			{
				GuiProperty::Input("Position", light.Position, 0.01f);
				GuiProperty::ColorEditHDR("Ambient", light.Ambient);
				GuiProperty::ColorEditHDR("Diffuse", light.Diffuse);
				GuiProperty::ColorEditHDR("Specular", light.Specular);
			});
		};

		GuiPropertyRAII::ID id(&scene.Light);
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load Light File", lightPathBuffer.data(), lightPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(lightPathBuffer.data(), scene.Light);

		lightGui("Character Light", scene.Light.Character, ImGuiTreeNodeFlags_DefaultOpen);
		lightGui("Stage Light", scene.Light.Stage, ImGuiTreeNodeFlags_DefaultOpen);
	}

	void SceneEditor::DrawIBLGui()
	{
		GuiPropertyRAII::ID id(&scene.IBL);
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load IBL File", iblPathBuffer.data(), iblPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(iblPathBuffer.data(), scene.IBL);

		GuiProperty::TreeNode("Lights", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			char nameBuffer[32];
			for (size_t i = 0; i < scene.IBL.Lights.size(); i++)
			{
				auto& lightData = scene.IBL.Lights[i];
				sprintf_s(nameBuffer, "Lights[%zu]", i);

				GuiPropertyRAII::ID id(&lightData);
				GuiProperty::TreeNode(nameBuffer, ImGuiTreeNodeFlags_DefaultOpen, [&]
				{
					GuiProperty::ColorEditHDR("Light Color", lightData.LightColor);
					// GuiProperty::Input("Light Direction", lightData.LightDirection, 0.01f);
				});
			}
		});

		GuiProperty::TreeNode("Light Maps", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			char nameBuffer[32];
			for (size_t i = 0; i < scene.IBL.LightMaps.size(); i++)
			{
				const auto& lightMap = scene.IBL.LightMaps[i];
				sprintf_s(nameBuffer, "Light Maps[%zu]", i);

				GuiPropertyRAII::ID id(&lightMap);
				GuiProperty::PropertyLabelValueFunc(nameBuffer, [&]
				{
					constexpr vec2 cubeMapDisplaySize = vec2(96.0f, 96.0f);

					const float width = std::clamp(Gui::GetContentRegionAvailWidth(), 1.0f, cubeMapDisplaySize.x);
					const vec2 size = vec2(width, width * (3.0f / 4.0f));

					ImTextureID textureID = lightMap;
					textureID.Data.CubeMapMipLevel = 0;
					Gui::Image(textureID, size);
					Gui::SameLine();
					textureID.Data.CubeMapMipLevel = 1;
					Gui::Image(textureID, size);

					return false;
				});
			}
		});
	}

	void SceneEditor::DrawSceneGraphGui()
	{
		Gui::BeginChild("SceneGraphEntiriesListChild");

		auto setCamera = [&](auto& entity, const Sphere& boundingSphere)
		{
			const Sphere transformedSphere = boundingSphere * entity->Transform;

			camera.Interest = camera.ViewPoint = transformedSphere.Center;
			cameraController.OrbitData.Distance = transformedSphere.Radius;

			inspector.EntityIndex = static_cast<int>(std::distance(&sceneGraph.Entities.front(), &entity));
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
		GuiPropertyRAII::PropertyValueColumns columns;

		auto entityNameGetter = [&](int i) { return InBounds(i, sceneGraph.Entities) ? sceneGraph.Entities[i]->Name.c_str() : "None"; };
		GuiProperty::ComboResult comboResult;
		GuiProperty::Combo("Entities", inspector.EntityIndex, -1, static_cast<int>(sceneGraph.Entities.size()), ImGuiComboFlags_HeightLarge, entityNameGetter, &comboResult);

		if (comboResult.IsOpen)
		{
			std::for_each(sceneGraph.Entities.begin(), sceneGraph.Entities.end(), [](auto& e) { e->Obj->Debug.UseDebugMaterial = false; });

			if (InBounds(comboResult.HoveredIndex, sceneGraph.Entities) && !comboResult.ValueChanged)
				sceneGraph.Entities[comboResult.HoveredIndex]->Obj->Debug.UseDebugMaterial = true;
		}

		if (!InBounds(inspector.EntityIndex, sceneGraph.Entities))
			return;

		auto& entity = *sceneGraph.Entities[inspector.EntityIndex];

		GuiPropertyRAII::ID id(&entity);
		GuiProperty::TreeNode("Entity", entity.Name, ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			GuiProperty::PropertyLabelValueFunc("Tag", [&] { return Gui::InputScalar(GuiProperty::Detail::DummyLabel, ImGuiDataType_U64, &entity.Tag, nullptr, nullptr, "0x%08X", ImGuiInputTextFlags_ReadOnly); });

			GuiProperty::Checkbox("Is Visible", entity.IsVisible);
			GuiProperty::Checkbox("Is Reflection", entity.IsReflection);
			GuiProperty::Checkbox("Silhouette Outline", entity.SilhouetteOutline);

			GuiProperty::Input("Translation", entity.Transform.Translation, 0.1f);
			GuiProperty::Input("Scale", entity.Transform.Scale, 0.05f);
			GuiProperty::Input("Rotation", entity.Transform.Rotation, 1.0f);
		});
	}

	void SceneEditor::DrawObjectTestGui()
	{
		auto collectionComboDebugMaterialHover = [&](std::string_view label, auto& collection, int& inOutIndex)
		{
			GuiProperty::ComboResult comboResult;
			GuiProperty::Combo(label, inOutIndex, -1, static_cast<int>(collection.size()), ImGuiComboFlags_HeightLarge, [&](int i) { return InBounds(i, collection) ? &collection[i].Name[0] : "None"; }, &comboResult);

			if (comboResult.IsOpen)
			{
				std::for_each(collection.begin(), collection.end(), [](auto& e) { e.Debug.UseDebugMaterial = false; });

				if (InBounds(comboResult.HoveredIndex, collection) && !comboResult.ValueChanged)
					collection[comboResult.HoveredIndex].Debug.UseDebugMaterial = true;
			}

			return InBounds(inOutIndex, collection);
		};

		GuiPropertyRAII::PropertyValueColumns columns;

		Obj* selectedObj = nullptr;
		GuiProperty::TreeNode("Object Select", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			GuiProperty::Combo("Obj Set", objTestData.ObjSetIndex, -1, static_cast<int>(sceneGraph.LoadedObjSets.size()), ImGuiComboFlags_HeightLarge, [&](int i)
			{
				return InBounds(i, sceneGraph.LoadedObjSets) ? sceneGraph.LoadedObjSets[i].ObjSet->Name.c_str() : "None";
			});

			if (!InBounds(objTestData.ObjSetIndex, sceneGraph.LoadedObjSets))
				return;

			ObjSet& objSet = *sceneGraph.LoadedObjSets[objTestData.ObjSetIndex].ObjSet;
			if (!collectionComboDebugMaterialHover("Obj", objSet, objTestData.ObjIndex))
				return;

			selectedObj = &objSet[objTestData.ObjIndex];
		});

		if (selectedObj == nullptr)
			return;

		GuiProperty::TreeNode("Material Editor", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			if (!collectionComboDebugMaterialHover("Material", selectedObj->Materials, objTestData.MaterialIndex))
				return;

			Material& material = selectedObj->Materials[objTestData.MaterialIndex];
			materialEditor.Gui(*renderer3D, scene, material);
		});

		GuiProperty::TreeNode("Mesh Flags Editor", [&]
		{
			if (!collectionComboDebugMaterialHover("Mesh", selectedObj->Meshes, objTestData.MeshIndex))
				return;

			Mesh& selectedMesh = selectedObj->Meshes[objTestData.MeshIndex];
			GuiProperty::TreeNode("Mesh", selectedMesh.Name.data(), ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiPropertyRAII::ID id(&selectedMesh);
				GuiProperty::Checkbox("Use Debug Material", selectedMesh.Debug.UseDebugMaterial);
				GuiProperty::Checkbox("Render Bounding Sphere", selectedMesh.Debug.RenderBoundingSphere);
				GuiPropertyBitFieldCheckbox("Face Camera", selectedMesh.Flags.FaceCameraPosition);
				GuiPropertyBitFieldCheckbox("Face Camera View", selectedMesh.Flags.FaceCameraView);
			});

			GuiProperty::TreeNode("Sub Meshes", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				for (size_t i = 0; i < selectedMesh.SubMeshes.size(); i++)
				{
					char nodePropertyBuffer[32];
					sprintf_s(nodePropertyBuffer, "Sub Meshes[%zu]", i);

					GuiProperty::TreeNode(nodePropertyBuffer, ImGuiTreeNodeFlags_DefaultOpen, [&]
					{
						SubMesh& subMesh = selectedMesh.SubMeshes[i];
						GuiPropertyRAII::ID id(&subMesh);
						GuiProperty::Checkbox("Use Debug Material", subMesh.Debug.UseDebugMaterial);
						GuiPropertyBitFieldCheckbox("Receives Shadows", subMesh.Flags.ReceivesShadows);
						GuiPropertyBitFieldCheckbox("Casts Shadows", subMesh.Flags.CastsShadows);
						GuiPropertyBitFieldCheckbox("Transparent", subMesh.Flags.Transparent);
					});
				}
			});
		});
	}

	void SceneEditor::DrawStageTestGui()
	{
		GuiPropertyRAII::PropertyValueColumns columns;

		for (auto& stageTypeData : stageTestData.TypeData)
		{
			auto load = [&]()
			{
				stageTypeData.ID = std::clamp(stageTypeData.ID, stageTypeData.MinID, stageTypeData.MaxID);
				stageTypeData.SubID = std::clamp(stageTypeData.SubID, 1, 39);

				if (stageTestData.Settings.LoadObj)
				{
					UnLoadStageObjects();
					LoadStageObjects(stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID, stageTestData.Settings.LoadLightParam);
					SetStageVisibility(StageVisibilityType::GroundSky);
				}
				else if (stageTestData.Settings.LoadLightParam)
				{
					Debug::LoadStageLightParamFiles(scene, stageTypeData.Type, stageTypeData.ID, stageTypeData.SubID);
				}

				stageTestData.lastSetStage.emplace(stageTypeData);
			};

			GuiPropertyRAII::ID id(&stageTypeData.ID);
			GuiProperty::PropertyLabelValueFunc(stageTypeData.Name, [&]
			{
				if (Gui::Button("Reload"))
					load();
				Gui::SameLine();
				if (Gui::InputInt(GuiProperty::Detail::DummyLabel, &stageTypeData.ID, 1, 10))
				{
					stageTypeData.SubID = 1;
					load();
				}
				return false;
			});

			if (stageTypeData.Type == StageType::STGPV)
			{
				char subIDLabelBuffer[32];
				sprintf_s(subIDLabelBuffer, "%.*s (Sub ID)", static_cast<int>(stageTypeData.Name.size()), stageTypeData.Name.data());

				GuiPropertyRAII::ID id(&stageTypeData.SubID);
				GuiProperty::PropertyLabelValueFunc(subIDLabelBuffer, [&]
				{
					if (Gui::Button("Reload"))
						load();
					Gui::SameLine();
					if (Gui::InputInt(GuiProperty::Detail::DummyLabel, &stageTypeData.SubID))
						load();
					return false;
				});
			}
		}

		GuiProperty::Checkbox("Load Light Param", stageTestData.Settings.LoadLightParam);
		GuiProperty::Checkbox("Load Stage Obj", stageTestData.Settings.LoadObj);

		static constexpr std::array visibilityButtons =
		{
			std::make_pair("Show All", StageVisibilityType::All),
			std::make_pair("Hide All", StageVisibilityType::None),
			std::make_pair("Show Ground & Sky", StageVisibilityType::GroundSky),
		};

		for (const auto& button : visibilityButtons)
		{
			if (GuiProperty::PropertyLabelValueFunc("Set Visibility", [&] { return Gui::Button(button.first, vec2(Gui::GetContentRegionAvailWidth() * 0.8f, 0.0f)); }))
				SetStageVisibility(button.second);
		}
	}

	void SceneEditor::DrawCharaTestGui()
	{
		auto unloadCharaItems = [&] { EraseByTag(CharacterTag, static_cast<EraseFlags>(EraseFlags_Entities | EraseFlags_ObjSets)); };

		auto loadCharaItems = [&]()
		{
			auto loadPart = [&](int id, int exclusiveObjIndex = -1, bool isCommonItem = false)
			{
				auto[objPathType, texPathType] = (isCommonItem) ?
					std::make_pair(Debug::PathType::CmnItemObj, Debug::PathType::CmnItemTex) :
					std::make_pair(Debug::PathType::CharaItemObj, Debug::PathType::CharaItemTex);

				auto objSetPath = Debug::GetDebugFilePath(objPathType, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());
				auto texSetPath = Debug::GetDebugFilePath(texPathType, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());

				if (LoadRegisterObjSet(objSetPath.data(), texSetPath.data(), CharacterTag))
				{
					auto& loadedResource = sceneGraph.LoadedObjSets.back();

					if (InBounds(exclusiveObjIndex, *loadedResource.ObjSet))
					{
						sceneGraph.AddEntityFromObj(loadedResource.ObjSet->at(exclusiveObjIndex), CharacterTag);
					}
					else
					{
						for (auto& obj : *loadedResource.ObjSet)
							sceneGraph.AddEntityFromObj(obj, CharacterTag);
					}
				}
			};

			unloadCharaItems();
			loadPart(charaTestData.IDs.CommonItem, -1, true);
			loadPart(charaTestData.IDs.Face, charaTestData.IDs.FaceIndex);
			loadPart(charaTestData.IDs.Overhead);
			loadPart(charaTestData.IDs.Hair);
			loadPart(charaTestData.IDs.Outer);
			loadPart(charaTestData.IDs.Hands);
		};

		GuiPropertyRAII::ID id(&charaTestData);
		GuiPropertyRAII::PropertyValueColumns columns;

		GuiProperty::Input("Character", charaTestData.IDs.Character.data(), charaTestData.IDs.Character.size());

		if (GuiProperty::Input("Item", charaTestData.IDs.CommonItem))
			loadCharaItems();

		if (GuiProperty::Input("Face", charaTestData.IDs.FaceIndex))
			loadCharaItems();

		if (GuiProperty::Input("Overhead", charaTestData.IDs.Overhead))
			loadCharaItems();

		if (GuiProperty::Input("Hair", charaTestData.IDs.Hair))
			loadCharaItems();

		if (GuiProperty::Input("Outer", charaTestData.IDs.Outer))
			loadCharaItems();

		if (GuiProperty::Input("Hands", charaTestData.IDs.Hands))
			loadCharaItems();

		bool transformChanged = false;
		transformChanged |= GuiProperty::Input("Translation", charaTestData.Transform.Translation, 0.1f);
		transformChanged |= GuiProperty::Input("Scale", charaTestData.Transform.Scale, 0.05f);
		transformChanged |= GuiProperty::Input("Rotation", charaTestData.Transform.Rotation, 1.0f);

		if (transformChanged)
		{
			for (auto& entity : sceneGraph.Entities)
			{
				if (entity->Tag == CharacterTag)
					entity->Transform = charaTestData.Transform;
			}
		}

		GuiProperty::PropertyFuncValueFunc([&]
		{
			if (Gui::Button("Reload", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
				loadCharaItems();
			return false;
		}, [&]
		{
			if (Gui::Button("Unload", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
				unloadCharaItems();
			return false;
		});
	}

	void SceneEditor::DrawA3DTestGui()
	{
#if COMFY_DEBUG && 0 // DEBUG:
#if 1
		scene.LensFlare.SunPosition = vec3(11.017094f, 5.928364f, -57.304039f);
		//scene.LensFlare.SunPosition = vec3(0.0f, 2.0f, 0.0f);
#endif

#if 1
		static constexpr std::string_view effCmnObjSetPath = "dev_rom/objset/copy/effcmn/effcmn_obj.bin";
		static constexpr std::string_view effCmnTexSetPath = "dev_rom/objset/copy/effcmn/effcmn_tex.bin";

		static std::unique_ptr<ObjSet> effCmnObjSet = nullptr;

		if (effCmnObjSet == nullptr)
		{
			effCmnObjSet = ObjSet::MakeUniqueReadParseUpload(effCmnObjSetPath);
			effCmnObjSet->TexSet = TexSet::MakeUniqueReadParseUpload(effCmnTexSetPath, effCmnObjSet.get());

			if (auto sunObj = std::find_if(effCmnObjSet->begin(), effCmnObjSet->end(), [&](const auto& obj) { return obj.Name == "effcmn_sun"; }); sunObj != effCmnObjSet->end())
				scene.LensFlare.SunObj = &(*sunObj);
		}
#elif 1
		scene.LensFlare.SunObj = nullptr;

		for (auto& objSet : sceneGraph.LoadedObjSets)
		{
			for (auto& obj : *objSet.ObjSet)
			{
				if (Utilities::EndsWithInsensitive(obj.Name, "lensflare_0"))
					scene.LensFlare.SunObj = &obj;
			}
		}
#endif
#endif /* COMFY_DEBUG */

#if 0 // DEBUG:
		// TODO:
		struct TestViewport
		{
			CameraController3D CameraController;
			SceneViewport Viewport;
		};

		static std::vector<std::unique_ptr<TestViewport>> testViewports;

		if (Gui::Button("Add Viewport"))
		{
			testViewports.push_back(std::move(std::make_unique<TestViewport>()));
			auto& testViewport = testViewports.back()->Viewport;
			testViewport.Camera = this->camera;
			testViewport.Parameters = this->viewport.Parameters;
			testViewport.Parameters.RenderResolution = ivec2(512, 288);
		}

		for (auto& testViewport : testViewports)
		{
			char viewportNameBuffer[64];
			sprintf_s(viewportNameBuffer, "Test Viewport (0x%p)", testViewport.get());

			bool isOpen = true;
			Gui::SetNextWindowSize(vec2(testViewport->Viewport.Parameters.RenderResolution), ImGuiCond_FirstUseEver);
			if (Gui::Begin(viewportNameBuffer, &isOpen, (ImGuiWindowFlags_NoSavedSettings)))
			{
				vec2 size = Gui::GetWindowSize();
				testViewport->Viewport.Camera.AspectRatio = size.x / size.y;
				testViewport->Viewport.Parameters.RenderResolution = size;
				testViewport->Viewport.Parameters.AutoExposure = false;
				testViewport->Viewport.Parameters.ToneMapPreserveAlpha = true;

				GuiPropertyRAII::ID id(&testViewport->Viewport);
				Gui::BeginChild("TestViewportChild");

				if (Gui::IsWindowFocused())
					testViewport->CameraController.Update(testViewport->Viewport.Camera);

				testViewport->Viewport.Camera.UpdateMatrices();
				renderer3D->Begin(testViewport->Viewport, scene);
				{
					for (const auto& entity : sceneGraph.Entities)
					{
						if (!entity->IsVisible)
							continue;

						RenderCommand renderCommand;
						renderCommand.SourceObj = entity->Obj;
						renderCommand.SourceMorphObj = entity->MorphObj;
						renderCommand.Transform = entity->Transform;
						renderCommand.Flags.IsReflection = entity->IsReflection;
						renderCommand.Animation = entity->Dynamic.get();

						if (entity->SilhouetteOutline)
							renderCommand.Flags.SilhouetteOutline = true;
						if (entity->Tag == 'chr' || entity->Tag == 'obj')
							renderCommand.Flags.CastsShadow = true;

						renderer3D->Draw(renderCommand);
					}
				}
				renderer3D->End();

				Gui::GetWindowDrawList()->AddImage(testViewport->Viewport.Data.Output.RenderTarget, Gui::GetWindowPos(), Gui::GetWindowPos() + size);
				Gui::EndChild();
			}
			Gui::End();

			if (!isOpen)
			{
				testViewports.erase(testViewports.begin() + std::distance(&testViewports.front(), &testViewport));
				break;
			}
		}
#endif
	}

	void SceneEditor::DrawExternalProcessTestGui()
	{
		auto tryAttach = [&]
		{
			if (externalProcessTest.ShouldReadConfigFile)
			{
				externalProcessTest.ShouldReadConfigFile = false;

				std::vector<u8> fileBuffer;
				if (ComfyData->ReadFileIntoBuffer("process/external_process.bin", fileBuffer))
					externalProcessTest.ExternalProcess.ParseConfig(fileBuffer.data(), fileBuffer.size());
				else
					externalProcessTest.WasConfigInvalid = true;
			}

			if (!externalProcessTest.ExternalProcess.IsAttached())
				externalProcessTest.ExternalProcess.Attach();

			return externalProcessTest.ExternalProcess.IsAttached();
		};

		if (externalProcessTest.WasConfigInvalid)
		{
			Gui::TextUnformatted("Invalid Config Error");
			return;
		}

		Gui::BeginChild("ExternalProcessChild", vec2(0.0f), true);
		{
			if (Gui::WideTreeNodeEx(ICON_FA_INFO "  Process Info", ImGuiTreeNodeFlags_DefaultOpen))
			{
				const auto& processData = externalProcessTest.ExternalProcess.GetProcess();
				const auto& processSettings = externalProcessTest.ExternalProcess.GetSettings();

				Gui::BeginColumns("ProcessInfoColumns", 2, ImGuiColumnsFlags_NoBorder);
				{
					Gui::TextUnformatted("Process Name");
					Gui::NextColumn();
					Gui::Text("%s", processSettings.ProcessName.c_str());
					Gui::NextColumn();

					Gui::TextUnformatted("Process ID");
					Gui::NextColumn();
					Gui::Text("%d", processData.ID);
					Gui::NextColumn();

					Gui::TextUnformatted("Process Handle");
					Gui::NextColumn();
					Gui::Text("%p", processData.Handle);
					Gui::NextColumn();
				}
				Gui::EndColumns();
				Gui::TreePop();
			}
			Gui::Separator();

			if (Gui::WideTreeNodeEx(ICON_FA_CAMERA "  Camera", ImGuiTreeNodeFlags_DefaultOpen))
			{
				Gui::BeginColumns("CameraColumns", 2, ImGuiColumnsFlags_NoBorder);
				{
					if (Gui::Checkbox("Sync Read Camera", &externalProcessTest.SyncReadCamera) && externalProcessTest.SyncReadCamera)
						cameraController.Mode = CameraController3D::ControlMode::None;
					Gui::NextColumn();
					if (Gui::Checkbox("Sync Write Camera", &externalProcessTest.SyncWriteCamera) && externalProcessTest.SyncWriteCamera)
						cameraController.Mode = CameraController3D::ControlMode::Orbit;
					Gui::NextColumn();
				}
				Gui::EndColumns();
				Gui::TreePop();
			}
			Gui::Separator();

			if (Gui::WideTreeNodeEx(ICON_FA_LIGHTBULB "  Light Param", ImGuiTreeNodeFlags_DefaultOpen))
			{
				Gui::BeginColumns("LightParamColumns", 2, ImGuiColumnsFlags_NoBorder);
				{
					Gui::Checkbox("Sync Read Light", &externalProcessTest.SyncReadLightParam);
					Gui::NextColumn();
					Gui::Checkbox("Sync Write Light", &externalProcessTest.SyncWriteLightParam);
					Gui::NextColumn();
				}
				Gui::EndColumns();
				Gui::TreePop();
			}
			Gui::Separator();
		}
		Gui::EndChild();

		if (externalProcessTest.SyncReadCamera && tryAttach())
		{
			const auto cameraData = externalProcessTest.ExternalProcess.ReadCamera();
			camera.ViewPoint = cameraData.ViewPoint;
			camera.Interest = cameraData.Interest;
			camera.FieldOfView = cameraData.FieldOfView;
		}
		else if (externalProcessTest.SyncWriteCamera && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteCamera({ camera.ViewPoint, camera.Interest, 0.0f, camera.FieldOfView });
		}

		if (externalProcessTest.SyncReadLightParam && tryAttach())
		{
			const auto lightData = externalProcessTest.ExternalProcess.ReadLightParam();

			scene.Light.Character.Ambient = lightData.Character.Ambient;
			scene.Light.Character.Diffuse = lightData.Character.Diffuse;
			scene.Light.Character.Specular = lightData.Character.Specular;
			scene.Light.Character.Position = lightData.Character.Position;

			scene.Light.Stage.Ambient = lightData.Stage.Ambient;
			scene.Light.Stage.Diffuse = lightData.Stage.Diffuse;
			scene.Light.Stage.Specular = lightData.Stage.Specular;
			scene.Light.Stage.Position = lightData.Stage.Position;

			scene.IBL.Lights[0].LightColor = lightData.IBL0.LightColor;
			scene.IBL.Lights[0].IrradianceRGB = lightData.IBL0.IrradianceRGB;
			scene.IBL.Lights[1].LightColor = lightData.IBL1.LightColor;
			scene.IBL.Lights[1].IrradianceRGB = lightData.IBL1.IrradianceRGB;
		}
		else if (externalProcessTest.SyncWriteLightParam && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteLightParam(
				{
					vec4(scene.Light.Character.Ambient, 1.0f),
					vec4(scene.Light.Character.Diffuse, 1.0f),
					vec4(scene.Light.Character.Specular, 1.0f),
					scene.Light.Character.Position,

					vec4(scene.Light.Stage.Ambient, 1.0f),
					vec4(scene.Light.Stage.Diffuse, 1.0f),
					vec4(scene.Light.Stage.Specular, 1.0f),
					scene.Light.Stage.Position,

					scene.IBL.Lights[0].LightColor,
					scene.IBL.Lights[0].IrradianceRGB,
					scene.IBL.Lights[1].LightColor,
					scene.IBL.Lights[1].IrradianceRGB,
				});
		}
	}

	void SceneEditor::DrawDebugTestGui()
	{
#if COMFY_DEBUG && 0
		Gui::DEBUG_NOSAVE_WINDOW("Loaded Textures Test", [&]
		{
			sceneGraph.TexIDMap.Iterate([&](ResourceIDMap<TexID, Tex>::ResourceIDPair& resourceIDPair)
			{
				TexID id = resourceIDPair.ID;
				Tex& tex = *resourceIDPair.Resource;

				char buffer[64];
				sprintf_s(buffer, "0x%X : %s", id, tex.GetName().data());
				Gui::Selectable(buffer, false);
				if (Gui::IsItemHovered())
				{
					Gui::BeginTooltip();
					Gui::ImageObjTex(&tex);
					Gui::EndTooltip();
				}
			});
		});
#endif

		static struct DebugData
		{
			static void ApplyA3DParentTransform(const A3DObject& parentObject, Transform& output, frame_t frame)
			{
				Transform parentTransform = A3DMgr::GetTransformAt(parentObject.Transform, frame);

				if (parentObject.Parent != nullptr)
					ApplyA3DParentTransform(*parentObject.Parent, parentTransform, frame);

				output.ApplyParent(parentTransform);
			}

			float MorphWeight = 0.0f, PlaybackSpeed = 0.0f, Elapsed = 0.0f;

			std::vector<std::unique_ptr<A3D>> StageEffA3Ds, CamPVA3Ds;
			int StageEffIndex = -1, CamPVIndex = -1;

			frame_t Frame = 0.0f, Duration = 1.0f, FrameRate = 60.0f;
			bool Playback = true, SetLongestDuration = true, Repeat = true, ApplyStageAuth = true;

		} debug;

		if (Gui::CollapsingHeader("Morph Weight Test", ImGuiTreeNodeFlags_None))
		{
			if (debug.PlaybackSpeed > 0.0f)
				debug.MorphWeight = glm::sin((debug.Elapsed += (Gui::GetIO().DeltaTime * debug.PlaybackSpeed))) + 1.0f;
			Gui::SliderFloat("Morph Weight", &debug.MorphWeight, 0.0f, 4.0f);
			Gui::SliderFloat("Playback Speed", &debug.PlaybackSpeed, 0.0f, 10.0f);

			for (size_t i = 0; i < sceneGraph.Entities.size(); i++)
			{
				auto* entity = sceneGraph.Entities[i].get();
				auto* nextEntity = InBounds(i + 1, sceneGraph.Entities) ? sceneGraph.Entities[i + 1].get() : nullptr;

				if (!EndsWith(entity->Name, "000") || nextEntity == nullptr)
					continue;

				if (entity->Dynamic == nullptr)
					entity->Dynamic = std::make_unique<Render::RenderCommand3D::DynamicData>();

				entity->Dynamic->MorphObj = nextEntity->Obj;
				entity->Dynamic->MorphWeight = debug.MorphWeight;

				int lastMorphIndex = 0;
				while ((entity = ((i + 1) < sceneGraph.Entities.size()) ? sceneGraph.Entities[++i].get() : nullptr) != nullptr)
				{
					int morphIndex = lastMorphIndex;
					sscanf_s(entity->Name.substr(entity->Name.size() - strlen("000")).data(), "%03d", &morphIndex);

					if (morphIndex == 0)
					{
						i--;
						break;
					}

					entity->IsVisible = false;
				}

				continue;
			}

			for (auto& entity : sceneGraph.Entities)
			{
				if (entity->Dynamic == nullptr || entity->Dynamic->MorphObj == nullptr)
					continue;

				if (entity->Obj->Meshes.size() != entity->Dynamic->MorphObj->Meshes.size())
					entity->Dynamic->MorphObj = nullptr;
			}
		}

		if (Gui::CollapsingHeader("A3D Test", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto loadA3Ds = [](const char* farcPath)
			{
				std::vector<std::unique_ptr<A3D>> a3ds;
				if (auto farc = IO::FArc::Open(farcPath); farc != nullptr)
				{
					for (const auto& file : farc->GetEntries())
					{
						auto content = file.ReadArray();
						a3ds.emplace_back(std::make_unique<A3D>())->Parse(content.get(), file.OriginalSize);
					}
				}
				return a3ds;
			};

			auto loadCamPVA3Ds = [loadA3Ds](int pvID)
			{
				char path[MAX_PATH]; sprintf_s(path, "dev_rom/auth_3d/CAMPV%03d.farc", pvID);
				return loadA3Ds(path);
			};

			auto loadStgEffA3Ds = [loadA3Ds](StageType type, int id)
			{
				char path[MAX_PATH]; sprintf_s(path, "dev_rom/auth_3d/EFFSTG%s%03d.farc", std::array { "TST", "NS", "D2NS", "PV" }[static_cast<int>(type)], id);
				return loadA3Ds(path);
			};

			// TODO: Optimize and refactor into its own A3DSceneManager (?) class
			auto applyA3D = [](auto& a3d, frame_t frame, auto& sceneGraph, auto& scene, auto& camera)
			{
				for (auto& object : a3d.Objects)
				{
					auto& entities = sceneGraph.Entities;
					auto correspondingEntity = std::find_if(entities.begin(), entities.end(), [&](auto& e) { return MatchesInsensitive(e->Name, object.UIDName); });

					if (correspondingEntity == entities.end())
						continue;

					auto& entity = *(correspondingEntity);
					entity->Transform = A3DMgr::GetTransformAt(object.Transform, frame);
					entity->IsVisible = A3DMgr::GetBoolAt(object.Transform.Visibility, frame);

					if (object.Parent != nullptr)
						DebugData::ApplyA3DParentTransform(*object.Parent, entity->Transform, frame);

					if (entity->Dynamic == nullptr)
						entity->Dynamic = std::make_unique<Render::RenderCommand3D::DynamicData>();

					// TODO: Instead of searching at the entire TexDB for entries only the loaded ObjSets would have to be checked (?)
					//		 As long as their Texs have been updated using a TexDB before that is

					if (!object.TexturePatterns.empty())
					{
						const auto& a3dPatterns = object.TexturePatterns;
						auto& entityPatterns = entity->Dynamic->TexturePatterns;

						if (entityPatterns.size() != a3dPatterns.size())
							entityPatterns.resize(a3dPatterns.size());

						for (size_t i = 0; i < a3dPatterns.size(); i++)
						{
							const auto& a3dPattern = a3dPatterns[i];
							auto& entityPattern = entityPatterns[i];

							if (a3dPattern.Pattern != nullptr)
							{
								if (!entityPattern.CachedIDs.has_value())
								{
									auto& cachedIDs = entityPattern.CachedIDs.emplace();
									cachedIDs.reserve(16);

									for (int cacheIndex = 0; cacheIndex < 999; cacheIndex++)
									{
										char nameBuffer[128];
										sprintf_s(nameBuffer, "%.*s_%03d", static_cast<int>(a3dPattern.Name.size() - strlen("_000")), a3dPattern.Name.data(), cacheIndex);

										auto texEntry = sceneGraph.TexDB->GetTexEntry(nameBuffer);
										if (texEntry == nullptr)
											break;

										if (cacheIndex == 0)
											entityPattern.OverrideID = texEntry->ID;

										cachedIDs.push_back(texEntry->ID);
									}
								}

								const int index = A3DMgr::GetIntAt(a3dPattern.Pattern->CV, frame);
								entityPattern.OverrideID = (InBounds(index, *entityPattern.CachedIDs)) ? entityPattern.CachedIDs->at(index) : TexID::Invalid;
							}
						}
					}

					if (!object.TextureTransforms.empty())
					{
						auto& a3dTexTransforms = object.TextureTransforms;
						auto& entityTexTransforms = entity->Dynamic->TextureTransforms;

						if (entityTexTransforms.size() != a3dTexTransforms.size())
							entityTexTransforms.resize(a3dTexTransforms.size());

						for (size_t i = 0; i < a3dTexTransforms.size(); i++)
						{
							auto& a3dTexTransform = a3dTexTransforms[i];
							auto& entityTexTransform = entityTexTransforms[i];

							if (entityTexTransform.SourceID == TexID::Invalid)
							{
								if (auto texEntry = sceneGraph.TexDB->GetTexEntry(a3dTexTransform.Name); texEntry != nullptr)
									entityTexTransform.SourceID = texEntry->ID;
							}

							// TODO: Might not be a single bool but different types
							if (a3dTexTransform.RepeatU.Enabled)
								entityTexTransform.RepeatU.emplace(A3DMgr::GetBoolAt(a3dTexTransform.RepeatU, frame));
							if (a3dTexTransform.RepeatV.Enabled)
								entityTexTransform.RepeatV.emplace(A3DMgr::GetBoolAt(a3dTexTransform.RepeatV, frame));

							entityTexTransform.Rotation = A3DMgr::GetRotationAt(a3dTexTransform.Rotate, frame) + A3DMgr::GetRotationAt(a3dTexTransform.RotateFrame, frame);

							entityTexTransform.Translation.x = A3DMgr::GetValueAt(a3dTexTransform.OffsetU, frame) - A3DMgr::GetValueAt(a3dTexTransform.TranslateFrameU, frame);
							entityTexTransform.Translation.y = A3DMgr::GetValueAt(a3dTexTransform.OffsetV, frame) - A3DMgr::GetValueAt(a3dTexTransform.TranslateFrameV, frame);
						}
					}

					if (object.Morph != nullptr)
					{
						size_t morphEntityIndex = static_cast<size_t>(std::distance(entities.begin(), correspondingEntity)) + 1;

						entity->Dynamic->MorphObj = (morphEntityIndex >= entities.size()) ? nullptr : entities[morphEntityIndex]->Obj;
						entity->Dynamic->MorphWeight = A3DMgr::GetValueAt(object.Morph->CV, frame);
					}
				}

				for (auto& a3dCamera : a3d.CameraRoot)
				{
					camera.ViewPoint = A3DMgr::GetValueAt(a3dCamera.ViewPoint.Transform.Translation, frame);
					camera.Interest = A3DMgr::GetValueAt(a3dCamera.Interest.Translation, frame);
					camera.FieldOfView = A3DMgr::GetFieldOfViewAt(a3dCamera.ViewPoint, frame);
				}
			};

			auto iterateA3Ds = [](std::vector<std::unique_ptr<A3D>>& a3ds, int index, auto func)
			{
				if (a3ds.empty())
					return;
				else if (index < 0)
					for (auto& a3d : a3ds)
						func(*a3d);
				else if (index < a3ds.size())
					func(*a3ds[index]);
			};

			const float availWidth = Gui::GetContentRegionAvailWidth();

			if (Gui::Button("Load STG EFF", vec2(availWidth * 0.25f, 0.0f)) && stageTestData.lastSetStage.has_value())
				debug.StageEffA3Ds = loadStgEffA3Ds(stageTestData.lastSetStage->Type, stageTestData.lastSetStage->ID);
			Gui::SameLine();
			if (Gui::Button("Unload EFF", vec2(availWidth * 0.25f, 0.0f)))
			{
				debug.StageEffA3Ds.clear();
				debug.StageEffIndex = -1;
			}

			if (Gui::Button("Load CAM PV ", vec2(availWidth * 0.25f, 0.0f)) && stageTestData.lastSetStage.has_value() && stageTestData.lastSetStage.value().Type == StageType::STGPV)
				debug.CamPVA3Ds = loadCamPVA3Ds(stageTestData.lastSetStage->ID);
			Gui::SameLine();
			if (Gui::Button("Unload CAM", vec2(availWidth * 0.25f, 0.0f)))
			{
				debug.CamPVA3Ds.clear();
				debug.CamPVIndex = -1;
			}

			if (Gui::InputInt("STGEFF Index", &debug.StageEffIndex))
				debug.StageEffIndex = std::clamp(debug.StageEffIndex, -1, static_cast<int>(debug.StageEffA3Ds.size()));
			if (Gui::IsItemHovered())
			{
				Gui::BeginTooltip();
				iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [](auto& a3d) { Gui::Selectable(a3d.Metadata.FileName.c_str()); });
				Gui::EndTooltip();
			}

			if (Gui::InputInt("CAMPV Index", &debug.CamPVIndex))
				debug.CamPVIndex = std::clamp(debug.CamPVIndex, -1, static_cast<int>(debug.CamPVA3Ds.size()));
			if (Gui::IsItemHovered())
			{
				Gui::BeginTooltip();
				iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [](auto& a3d) { Gui::Selectable(a3d.Metadata.FileName.c_str()); });
				Gui::EndTooltip();
			}

			Gui::Checkbox("Apply Stage Auth", &debug.ApplyStageAuth);

			if (Gui::IsWindowFocused() && Gui::IsKeyPressed(Input::KeyCode_Space))
				debug.Playback ^= true;

			Gui::Checkbox("Playback", &debug.Playback);
			Gui::SameLine();
			Gui::Checkbox("Repeat", &debug.Repeat);

			Gui::InputFloat("Frame", &debug.Frame, 1.0f, 100.0f);
			Gui::InputFloat("Duration", &debug.Duration, 1.0f, 100.0f);

			if (debug.SetLongestDuration)
			{
				debug.Duration = 0.0f;
				iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { debug.Duration = std::max(debug.Duration, a3d.PlayControl.Duration); });
				iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { debug.Duration = std::max(debug.Duration, a3d.PlayControl.Duration); });
			}

			if (debug.Playback)
			{
				debug.Frame += 1.0f * (Gui::GetIO().DeltaTime * debug.FrameRate);
				if (debug.Repeat && debug.Frame >= debug.Duration)
					debug.Frame = 0.0f;
				debug.Frame = std::clamp(debug.Frame, 0.0f, debug.Duration);
			}

			if (debug.ApplyStageAuth)
				iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, camera); });

			if (cameraController.Mode == CameraController3D::ControlMode::None)
				iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, camera); });

			if (Gui::Button("Camera Mode Orbit")) { camera.FieldOfView = 90.0f; cameraController.Mode = CameraController3D::ControlMode::Orbit; }
			Gui::SameLine();
			if (Gui::Button("Camera Mode None")) { cameraController.Mode = CameraController3D::ControlMode::None; }

			if (Gui::Button("Set Screen Render IDs"))
			{
				for (auto& entity : sceneGraph.Entities)
				{
					for (const auto& material : entity->Obj->Materials)
					{
						const auto diffuseTexture = FindIfOrNull(material.Textures, [&](const auto& t) { return t.TextureFlags.Type == MaterialTextureType::ColorMap; });
						if (diffuseTexture == nullptr)
							continue;

						if (const auto* tex = renderer3D->GetTexFromTextureID(&diffuseTexture->TextureID); tex != nullptr && tex->Name.has_value())
						{
							if (const auto& name = tex->Name.value();
								EndsWithInsensitive(name, "_RENDER") ||
								EndsWithInsensitive(name, "_MOVIE") ||
								EndsWithInsensitive(name, "_TV") ||
								EndsWithInsensitive(name, "_FB01") ||
								EndsWithInsensitive(name, "_FB02") ||
								EndsWithInsensitive(name, "_FB03"))
							{
								if (entity->Dynamic == nullptr)
									entity->Dynamic = std::make_unique<Render::RenderCommand3D::DynamicData>();

								entity->Dynamic->ScreenRenderTextureID = diffuseTexture->TextureID;
							}
						}
					}
				}
			}
		}
	}

	void SceneEditor::TakeScreenshotGui()
	{
		const bool isScreenshotSaving = lastScreenshotTaskFuture.valid() && !lastScreenshotTaskFuture._Is_ready();
		const vec4 loadingColor = vec4(0.83f, 0.75f, 0.42f, 1.00f);

		if (isScreenshotSaving)
			Gui::PushStyleColor(ImGuiCol_Text, loadingColor);
		if (Gui::Button("Take Screenshot", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)) && renderWindow->GetRenderTarget() != nullptr)
			TakeSceneRenderTargetScreenshot(*renderWindow->GetRenderTarget());
		if (isScreenshotSaving)
			Gui::PopStyleColor(1);
		Gui::ItemContextMenu("TakeScreenshotContextMenu", [&]
		{
			if (Gui::MenuItem("Open Directory"))
				IO::Shell::OpenInExplorer(ScreenshotDirectoy);
		});

		constexpr const char* renderPopupID = "RenderSequencePopup";

		if (Gui::Button("Render Sequence...", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
			Gui::OpenPopup(renderPopupID);

		if (Gui::BeginPopup(renderPopupID))
		{
			static struct SequenceData
			{
				int FramesToRender = 360 / 6;
				float RotationXStep = 6.0f;
				std::vector<std::future<void>> Futures;
			} data;

			Gui::InputInt("Frams To Render", &data.FramesToRender);
			Gui::InputFloat("Rotation Step", &data.RotationXStep);

			if (Gui::Button("Render!", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)) && renderWindow->GetRenderTarget() != nullptr)
			{
				auto& renderTarget = *renderWindow->GetRenderTarget();

				data.Futures.clear();
				data.Futures.reserve(data.FramesToRender);

				for (int i = 0; i < data.FramesToRender; i += 1)
				{
					cameraController.Mode = CameraController3D::ControlMode::Orbit;
					cameraController.OrbitData.TargetRotation.x = static_cast<float>(i) * data.RotationXStep;
					cameraController.Update(camera);

					renderWindow->RenderScene();

					data.Futures.push_back(std::async(std::launch::async, [&renderTarget, i, data = std::move(renderTarget.StageAndCopyBackBuffer())]
						{
							char fileName[MAX_PATH];
							sprintf_s(fileName, "%s/sequence/scene_%04d.png", ScreenshotDirectoy, i);
							Utilities::WritePNG(fileName, renderTarget.Param.RenderResolution, data.get());
						}));
				}
			}
			Gui::EndPopup();
		}
	}

	void SceneEditor::TakeSceneRenderTargetScreenshot(Render::RenderTarget3D& renderTarget)
	{
		auto pixelData = renderTarget.StageAndCopyBackBuffer();

		if (lastScreenshotTaskFuture.valid())
			lastScreenshotTaskFuture.wait();

		lastScreenshotTaskFuture = std::async(std::launch::async, [&renderTarget, data = std::move(pixelData)]
			{
				const auto filePath = IO::Path::Combine(ScreenshotDirectoy, IO::Path::ChangeExtension("scene_" + FormatFileNameDateTimeNow(), ".png"));
				Utilities::WritePNG(filePath, renderTarget.Param.RenderResolution, data.get());
			});
	}
}

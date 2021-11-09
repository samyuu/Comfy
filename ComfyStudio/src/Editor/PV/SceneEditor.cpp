#include "SceneEditor.h"
#include "Debug.h"
#include "Graphics/Auth3D/A3D/A3D.h"
#include "Graphics/Auth3D/A3D/A3DMgr.h"
#include "Graphics/Auth3D/Misc/DebugObj.h"
#include "IO/Archive/FArc.h"
#include "IO/Shell.h"
#include "ImGui/Extensions/TexExtensions.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "ImGui/Widgets/ImageGridView.h"
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

	SceneEditor::SceneEditor(ComfyStudioApplication& parent, EditorManager& editor) : IEditorComponent(parent, editor)
	{
		auto texGetter = [&](const Cached_TexID* texID) { return sceneGraph.TexIDMap.Find(texID); };
		renderer3D = std::make_unique<Render::Renderer3D>(texGetter);

		mainViewport.RenderWindow = std::make_unique<SceneRenderWindow>(sceneGraph, mainViewport.Camera, *renderer3D, scene, mainViewport.CameraController);

		mainViewport.Camera.FieldOfView = 90.0f;
		mainViewport.Camera.ViewPoint = vec3(0.0f, 1.1f, 1.5f);
		mainViewport.Camera.Interest = vec3(0.0f, 1.0f, 0.0f);

		mainViewport.CameraController.FirstPersonData.TargetPitch = -11.0f;
		mainViewport.CameraController.FirstPersonData.TargetYaw = -90.000f;

		if (sceneGraph.LoadedObjSets.empty())
		{
			LoadStageObjects(StageType::STGTST, 6, 0);
			// LoadStageObjects(StageType::STGTST, 7, 0);
			// LoadStageObjects(StageType::STGNS, 8, 0);
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

		auto viewportGui = [&](ViewportContext& viewport, const char* name)
		{
			viewport.RenderWindow->BeginEndGui(name, &viewport.IsOpen);

			if (viewport.RenderWindow->GetRequestsDuplication())
				AddViewport(&viewport);

			if (const auto rayPickRequest = viewport.RenderWindow->GetRayPickRequest(); rayPickRequest.has_value())
				HandleRayPickRequest(viewport, *rayPickRequest);
		};

		viewportGui(mainViewport, ICON_FA_TREE "  Scene Viewport Main##SceneEditor");

		additionalViewports.erase(std::remove_if(additionalViewports.begin(), additionalViewports.end(), [](const auto& viewport) { return !viewport->IsOpen; }), additionalViewports.end());
		for (size_t i = 0; i < additionalViewports.size(); i++)
		{
			char nameBuffer[64]; sprintf_s(nameBuffer, ICON_FA_TREE "  Scene Viewport [%zu]##SceneEditor", i);
			viewportGui(*additionalViewports[i], nameBuffer);
		}

		auto& activeViewport = FindActiveViewport();

		if (Gui::Begin(ICON_FA_CAMERA "  Camera"))
			DrawCameraGui(activeViewport);
		Gui::End();

		if (Gui::Begin(ICON_FA_FOLDER "  ObjSet Loader"))
			DrawObjSetLoaderGui();
		Gui::End();

		if (Gui::Begin(ICON_FA_WRENCH "  Rendering"))
			DrawRenderingGui(activeViewport);
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
			DrawSceneGraphGui(activeViewport);
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
			DrawA3DTestGui(mainViewport);
		Gui::End();

		if (Gui::Begin(ICON_FA_PROJECT_DIAGRAM "  External Process"))
			DrawExternalProcessTestGui(mainViewport);
		Gui::End();

		if (Gui::Begin(ICON_FA_BUG "  Debug Test"))
			DrawDebugTestGui(mainViewport);
		Gui::End();
	}

	bool SceneEditor::LoadRegisterObjSet(std::string_view objSetPath, std::string_view texSetPath, EntityTag tag)
	{
		if (!IO::File::Exists(objSetPath) || !IO::File::Exists(texSetPath))
			return false;

		if (objSetPath == texSetPath)
			return false;

		std::shared_ptr<ObjSet> objSet = IO::File::Load<ObjSet>(objSetPath);
		if (objSet == nullptr)
			return false;

		objSet->Name = IO::Path::GetFileName(objSetPath, false);
		objSet->TexSet = TexSet::LoadSetTextureIDs(texSetPath, objSet.get());
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
		for (auto& obj : newlyAddedStageObjSet.ObjSet->Objects)
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

	void SceneEditor::AddViewport(ViewportContext* baseViewport)
	{
		auto& newViewport = *additionalViewports.emplace_back(std::move(std::make_unique<ViewportContext>()));
		newViewport.RenderWindow = std::make_unique<SceneRenderWindow>(sceneGraph, newViewport.Camera, *renderer3D, scene, newViewport.CameraController);

		if (baseViewport != nullptr)
		{
			newViewport.Camera = baseViewport->Camera;
			newViewport.RenderWindow->GetRenderTarget()->Param = baseViewport->RenderWindow->GetRenderTarget()->Param;
		}
	}

	SceneEditor::ViewportContext& SceneEditor::FindActiveViewport()
	{
		ViewportContext* newestFocusedViewport = &mainViewport;
		for (auto& viewport : additionalViewports)
		{
			if (viewport->RenderWindow != nullptr && viewport->RenderWindow->GetLastFocusedFrameCount() > newestFocusedViewport->RenderWindow->GetLastFocusedFrameCount())
				newestFocusedViewport = viewport.get();
		}

		return *newestFocusedViewport;
	}

	void SceneEditor::HandleRayPickRequest(const ViewportContext& viewport, const SceneRenderWindow::RayPickResult& rayPick)
	{
		if (rayPick.Entity == nullptr)
		{
			inspector.EntityIndex = -1;
			objTestData.ObjSetIndex = -1;
			objTestData.ObjIndex = -1;
			objTestData.MeshIndex = -1;
			objTestData.MaterialIndex = -1;
			return;
		}

		inspector.EntityIndex = static_cast<int>(FindIndexOf(sceneGraph.Entities, [&](const auto& entity) { return (entity.get() == rayPick.Entity); }));

		for (size_t setIndex = 0; setIndex < sceneGraph.LoadedObjSets.size(); setIndex++)
		{
			const auto& resource = sceneGraph.LoadedObjSets[setIndex];
			for (size_t objIndex = 0; objIndex < resource.ObjSet->Objects.size(); objIndex++)
			{
				const auto& obj = resource.ObjSet->Objects[objIndex];
				for (size_t meshIndex = 0; meshIndex < obj.Meshes.size(); meshIndex++)
				{
					const auto& mesh = obj.Meshes[meshIndex];
					for (size_t subMeshIndex = 0; subMeshIndex < mesh.SubMeshes.size(); subMeshIndex++)
					{
						const auto& subMesh = mesh.SubMeshes[subMeshIndex];
						if (&subMesh != rayPick.SubMesh)
							continue;

						objTestData.ObjSetIndex = static_cast<int>(setIndex);
						objTestData.ObjIndex = static_cast<int>(objIndex);
						objTestData.MeshIndex = static_cast<int>(meshIndex);
						objTestData.MaterialIndex = static_cast<int>(subMesh.MaterialIndex);
					}
				}
			}
		}
	}

	void SceneEditor::DrawCameraGui(ViewportContext& activeViewport)
	{
		auto& camera = activeViewport.Camera;
		auto& cameraController = activeViewport.CameraController;

		GuiPropertyRAII::PropertyValueColumns columns;
		GuiPropertyRAII::ID id(&camera);

		GuiProperty::Input("Orthographic Lerp", camera.OrthographicLerp, 0.01f, vec2(0.0f, 1.0f));
		GuiProperty::Input("Field Of View", camera.FieldOfView, 1.0f, vec2(1.0f, 173.0f));
		GuiProperty::Input("Near Plane", camera.NearPlane, 0.001f, (camera.OrthographicLerp == 0.0f) ? vec2(0.001f, 1.0f) : std::optional<vec2> {});
		GuiProperty::Input("Far Plane", camera.FarPlane, 10.0f);
		GuiProperty::Input("View Point", camera.ViewPoint, 0.01f);
		GuiProperty::Input("Interest", camera.Interest, 0.01f);

		auto controlMode = cameraController.Mode;
		if (GuiProperty::Combo("Control Mode", controlMode, CameraController3D::ControlModeNames, ImGuiComboFlags_None))
			cameraController.SetControlModePreserveOrientation(camera, controlMode);

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
			GuiProperty::Input("Camera Yaw", cameraController.FirstPersonData.TargetYaw, 1.0f);
			GuiProperty::Input("Camera Pitch", cameraController.FirstPersonData.TargetPitch, 1.0f);
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
				for (auto& obj : newlyAddedStageObjSet.ObjSet->Objects)
				{
					auto& entity = sceneGraph.AddEntityFromObj(obj, ObjectTag);
					entity.CastsShadow = true;
				}
			}
		}
		Gui::EndChild();
	}

	void SceneEditor::DrawRenderingGui(ViewportContext& activeViewport)
	{
		auto& activeRenderWindow = *activeViewport.RenderWindow;
		if (activeRenderWindow.GetRenderTarget() == nullptr)
			return;

		auto& renderParam = activeRenderWindow.GetRenderTarget()->Param;
		GuiPropertyRAII::ID id(&renderParam);

		TakeScreenshotGui(activeViewport);

		GuiProperty::TreeNode("Render Parameters", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, [&]
		{
			auto resolutionGui = [&](const char* label, const char* contextMenu, ivec2& inOutResolution)
			{
				auto clampValidTextureSize = [](ivec2 size) { return Clamp(size, Render::RenderTargetMinSize, Render::RenderTargetMaxSize); };

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
					for (const f32 factor : scaleFactors)
					{
						sprintf_s(nameBuffer, "Render Region x%.2f", factor);
						if (Gui::MenuItem(nameBuffer))
							inOutResolution = clampValidTextureSize(ivec2(vec2(activeRenderWindow.GetRenderRegion().GetSize()) * factor));
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

				GuiProperty::Checkbox("Visualize Sun Occlusion Query", renderParam.DebugVisualizeSunOcclusionQuery);
			});

			GuiProperty::TreeNode("General", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				resolutionGui("Main Scene", "RenderResolutionContextMenu", renderParam.RenderResolution);
				resolutionGui("Reflection", "ReflectionResolutionContextMenu", renderParam.ReflectionRenderResolution);

				if (GuiProperty::Input("Multi Sample Count", renderParam.MultiSampleCount))
					renderParam.MultiSampleCount = Clamp(renderParam.MultiSampleCount, 1u, 16u);

				if (GuiProperty::Input("Anistropic Filtering", renderParam.AnistropicFiltering))
					renderParam.AnistropicFiltering = Clamp(renderParam.AnistropicFiltering, Render::AnistropicFilteringMin, Render::AnistropicFilteringMax);

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
					renderParam.ShadowBlurPasses = Clamp(renderParam.ShadowBlurPasses, 0u, 10u);
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
			if (activeRenderWindow.GetRenderTarget() == nullptr)
				return;

			auto[subTargets, subTargetCount] = activeRenderWindow.GetRenderTarget()->GetSubTargets();
			for (i32 index = 0; index < static_cast<i32>(subTargetCount); index++)
			{
				const auto& subTarget = subTargets[index];
				Gui::PushID(&subTarget);

				const u64 openMask = (static_cast<u64>(1) << static_cast<u64>(index));
				Gui::Selectable(subTarget.Name.c_str());

				if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
					openRenderTargetsFlags ^= openMask;

				const f32 aspectRatio = (static_cast<f32>(subTarget.Size.y) / static_cast<f32>(subTarget.Size.x));
				if (openRenderTargetsFlags & openMask)
				{
					constexpr f32 desiredWidth = 512.0f;

					bool open = true;
					if (Gui::Begin(subTarget.Name.c_str(), &open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoDocking))
						Gui::Image(subTarget.TextureID, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::End();
					if (!open)
						openRenderTargetsFlags &= ~openMask;
				}
				else if (Gui::IsItemHovered())
				{
					constexpr f32 desiredWidth = 256.0f;

					Gui::BeginTooltip();
					Gui::Image(subTarget.TextureID, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::EndTooltip();
				}

				Gui::PopID();
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
		GuiProperty::Input("Flare Alpha", scene.Glow.FlareA, 0.005f, vec2(0.0f, 1.0f));
		GuiProperty::Input("Shaft Alpha", scene.Glow.ShaftA, 0.005f, vec2(0.0f, 1.0f));
		GuiProperty::Input("Ghost Alpha", scene.Glow.GhostA, 0.005f, vec2(0.0f, 1.0f));
	}

	void SceneEditor::DrawLightGui()
	{
		using LightGuiFlags = u32;
		enum LightGuiFlags_Enum : LightGuiFlags
		{
			LightGuiFlags_None = 0,
			LightGuiFlags_Position = 1 << 0,
			LightGuiFlags_Ambient = 1 << 1,
			LightGuiFlags_Diffuse = 1 << 2,
			LightGuiFlags_Specular = 1 << 3,
			LightGuiFlags_Spot = 1 << 4,

			LightGuiFlags_All = 0xFFFFFFFF,
		};

		auto lightGui = [](std::string_view name, Light& light, ImGuiTreeNodeFlags nodeFlags, LightGuiFlags guiFlags)
		{
			GuiPropertyRAII::ID id(&light);

			GuiProperty::TreeNode(name, nodeFlags, [&]
			{
				GuiProperty::Combo("Type", light.Type, LightSourceTypeNames);

				if (light.Type == LightSourceType::None)
					return;

				if (guiFlags & LightGuiFlags_Position)
					GuiProperty::Input("Position", light.Position, 0.01f);

				if (guiFlags & LightGuiFlags_Ambient)
					GuiProperty::ColorEditHDR("Ambient", light.Ambient);

				if (guiFlags & LightGuiFlags_Diffuse)
					GuiProperty::ColorEditHDR("Diffuse", light.Diffuse);

				if (guiFlags & LightGuiFlags_Specular)
					GuiProperty::ColorEditHDR("Specular", light.Specular);

				if (light.Type == LightSourceType::Spot && guiFlags & LightGuiFlags_Spot)
				{
					GuiProperty::Input("Spot Direction", light.SpotDirection, 0.01f);
					GuiProperty::Input("Spot Exponent", light.SpotExponent, 0.01f);
					GuiProperty::Input("Spot Cuttoff", light.SpotCuttoff, 0.01f);
					GuiProperty::Input("Attenuation Constant", light.AttenuationConstant, 0.01f);
					GuiProperty::Input("Attenuation Linear", light.AttenuationLinear, 0.01f);
					GuiProperty::Input("Attenuation Quadratic", light.AttenuationQuadratic, 0.01f);
				}
			});
		};

		GuiPropertyRAII::ID id(&scene.Light);
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load Light File", lightPathBuffer.data(), lightPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(lightPathBuffer.data(), scene.Light);

		lightGui("Character Light", scene.Light.Character, ImGuiTreeNodeFlags_DefaultOpen, LightGuiFlags_All & ~LightGuiFlags_Ambient);
		lightGui("Stage Light", scene.Light.Stage, ImGuiTreeNodeFlags_DefaultOpen, LightGuiFlags_All & ~LightGuiFlags_Ambient);
		lightGui("Stage Sun", scene.Light.Sun, ImGuiTreeNodeFlags_DefaultOpen, LightGuiFlags_Position | LightGuiFlags_Diffuse);
		lightGui("Character Shadow", scene.Light.Shadow, ImGuiTreeNodeFlags_DefaultOpen, LightGuiFlags_Ambient);
	}

	void SceneEditor::DrawIBLGui()
	{
		GuiPropertyRAII::ID id(scene.IBL.get());
		GuiPropertyRAII::PropertyValueColumns columns;

		if (GuiProperty::Input("Load IBL File", iblPathBuffer.data(), iblPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			scene.IBL = IO::File::Load<Graphics::IBLParameters>(iblPathBuffer.data());

		if (scene.IBL == nullptr)
			return;

		GuiProperty::TreeNode("Lights", ImGuiTreeNodeFlags_DefaultOpen, [&]
		{
			char nameBuffer[32];
			for (size_t i = 0; i < scene.IBL->Lights.size(); i++)
			{
				auto& lightData = scene.IBL->Lights[i];
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
			for (size_t i = 0; i < scene.IBL->LightMaps.size(); i++)
			{
				const auto& lightMap = scene.IBL->LightMaps[i];
				sprintf_s(nameBuffer, "Light Maps[%zu]", i);

				GuiPropertyRAII::ID id(&lightMap);
				GuiProperty::PropertyLabelValueFunc(nameBuffer, [&]
				{
					constexpr vec2 cubeMapDisplaySize = vec2(96.0f, 96.0f);

					const f32 width = Clamp(Gui::GetContentRegionAvail().x, 1.0f, cubeMapDisplaySize.x);
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

	void SceneEditor::DrawSceneGraphGui(ViewportContext& activeViewport)
	{
		Gui::BeginChild("SceneGraphEntiriesListChild");

		auto setCamera = [&](auto& entity, const Sphere& boundingSphere)
		{
			const Sphere transformedSphere = boundingSphere * entity->Transform;
			activeViewport.CameraController.FitOrbitAroundSphere(activeViewport.Camera, transformedSphere);

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
			GuiProperty::Checkbox("Casts Shadow", entity.CastsShadow);
			GuiProperty::Checkbox("Ignore Shadow Cast Obj Flags", entity.IgnoreShadowCastObjFlags);
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
			if (!collectionComboDebugMaterialHover("Obj", objSet.Objects, objTestData.ObjIndex))
				return;

			selectedObj = &objSet.Objects[objTestData.ObjIndex];
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
				stageTypeData.ID = Clamp(stageTypeData.ID, stageTypeData.MinID, stageTypeData.MaxID);
				stageTypeData.SubID = Clamp(stageTypeData.SubID, 1, 39);

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
			if (GuiProperty::PropertyLabelValueFunc("Set Visibility", [&] { return Gui::Button(button.first, vec2(Gui::GetContentRegionAvail().x * 0.8f, 0.0f)); }))
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

					if (InBounds(exclusiveObjIndex, loadedResource.ObjSet->Objects))
					{
						auto& entity = sceneGraph.AddEntityFromObj(loadedResource.ObjSet->Objects[exclusiveObjIndex], CharacterTag);
						entity.CastsShadow = true;
						entity.IgnoreShadowCastObjFlags = true;
					}
					else
					{
						for (auto& obj : loadedResource.ObjSet->Objects)
						{
							auto& entity = sceneGraph.AddEntityFromObj(obj, CharacterTag);
							entity.CastsShadow = true;
							entity.IgnoreShadowCastObjFlags = true;
						}
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

		Gui::ItemContextMenu("CharacterContextMenu", [&]
		{
			static constexpr std::array<decltype(charaTestData.IDs.Character), 3> avilableCharacters = { "rin", "mik", "luk" };

			Gui::TextUnformatted("Character Names:");

			char labelBuffer[260];
			for (const auto& characterName : avilableCharacters)
			{
				sprintf_s(labelBuffer, "Set: { \"%.*s\" }", static_cast<int>(characterName.size()), characterName.data());

				const bool enabled = (charaTestData.IDs.Character != characterName);
				if (Gui::MenuItem(labelBuffer, nullptr, false, enabled))
				{
					charaTestData.IDs.Character = characterName;
					loadCharaItems();
				}
			}

			Gui::Separator();
			Gui::TextUnformatted("ID Presets:");

			for (const auto& preset : CharacterTestItemPresets)
			{
				sprintf_s(labelBuffer, "Set: { %d, %d, %d, %d, %d, %d, %d { \"%.*s\" } }",
					preset.CommonItem, preset.Face, preset.FaceIndex, preset.Overhead, preset.Hair, preset.Outer, preset.Hands, static_cast<int>(preset.Character.size()), preset.Character.data());

				const bool enabled = (std::memcmp(&charaTestData.IDs, &preset, sizeof(charaTestData.IDs)) != 0);
				if (Gui::MenuItem(labelBuffer, nullptr, false, enabled))
				{
					charaTestData.IDs = preset;
					loadCharaItems();
				}
			}
		});

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
			if (Gui::Button("Reload", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				loadCharaItems();
			return false;
		}, [&]
		{
			if (Gui::Button("Unload", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
				unloadCharaItems();
			return false;
		});
	}

	void SceneEditor::DrawA3DTestGui(ViewportContext& activeViewport)
	{
		static struct DebugData
		{
			static void ApplyA3DParentTransform(const A3DObject& parentObject, Transform& output, frame_t frame)
			{
				Transform parentTransform = A3DMgr::GetTransformAt(parentObject.Transform, frame);

				if (parentObject.Parent != nullptr)
					ApplyA3DParentTransform(*parentObject.Parent, parentTransform, frame);

				output.ApplyParent(parentTransform);
			}

			f32 MorphWeight = 0.0f, PlaybackSpeed = 0.0f, Elapsed = 0.0f;

			std::vector<std::unique_ptr<A3D>> StageEffA3Ds, CamPVA3Ds;
			int StageEffIndex = -1, CamPVIndex = -1;

			frame_t Frame = 0.0f, Duration = 1.0f, FrameRate = 60.0f;
			bool Playback = true, SetLongestDuration = true, Repeat = true, ApplyStageAuth = true;

		} debug;

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
				auto correspondingEntity = std::find_if(entities.begin(), entities.end(), [&](auto& e) { return Util::MatchesInsensitive(e->Name, object.UIDName); });

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
					const auto morphEntityIndex = static_cast<size_t>(std::distance(entities.begin(), correspondingEntity)) + 1;

					entity->Dynamic->MorphObj = (InBounds(morphEntityIndex, entities)) ? entities[morphEntityIndex]->Obj : nullptr;
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

		const f32 availWidth = Gui::GetContentRegionAvail().x;

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
			debug.StageEffIndex = Clamp(debug.StageEffIndex, -1, static_cast<int>(debug.StageEffA3Ds.size()));
		if (Gui::IsItemHovered())
		{
			Gui::BeginTooltip();
			iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [](auto& a3d) { Gui::Selectable(a3d.Metadata.FileName.c_str()); });
			Gui::EndTooltip();
		}

		if (Gui::InputInt("CAMPV Index", &debug.CamPVIndex))
			debug.CamPVIndex = Clamp(debug.CamPVIndex, -1, static_cast<int>(debug.CamPVA3Ds.size()));
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
			iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { debug.Duration = Max(debug.Duration, a3d.PlayControl.Duration); });
			iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { debug.Duration = Max(debug.Duration, a3d.PlayControl.Duration); });
		}

		if (debug.Playback)
		{
			debug.Frame += 1.0f * (Gui::GetIO().DeltaTime * debug.FrameRate);
			if (debug.Repeat && debug.Frame >= debug.Duration)
				debug.Frame = 0.0f;
			debug.Frame = Clamp(debug.Frame, 0.0f, debug.Duration);
		}

		if (debug.ApplyStageAuth)
			iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, activeViewport.Camera); });

		if (activeViewport.CameraController.Mode == CameraController3D::ControlMode::None)
			iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, activeViewport.Camera); });

		if (Gui::Button("Camera Mode Orbit")) { activeViewport.Camera.FieldOfView = 90.0f; activeViewport.CameraController.Mode = CameraController3D::ControlMode::Orbit; }
		Gui::SameLine();
		if (Gui::Button("Camera Mode None")) { activeViewport.CameraController.Mode = CameraController3D::ControlMode::None; }

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
							Util::EndsWithInsensitive(name, "_RENDER") ||
							Util::EndsWithInsensitive(name, "_MOVIE") ||
							Util::EndsWithInsensitive(name, "_TV") ||
							Util::EndsWithInsensitive(name, "_FB01") ||
							Util::EndsWithInsensitive(name, "_FB02") ||
							Util::EndsWithInsensitive(name, "_FB03"))
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

	void SceneEditor::DrawExternalProcessTestGui(ViewportContext& activeViewport)
	{
		auto tryAttach = [&]
		{
			if (externalProcessTest.ShouldReadConfigFile)
			{
				externalProcessTest.ShouldReadConfigFile = false;

				if (const auto configEntry = System::Data.FindFile("process/external_process.bin"); configEntry != nullptr)
				{
					auto configBuffer = std::vector<u8>(configEntry->Size);
					if (System::Data.ReadFileIntoBuffer(configEntry, configBuffer.data()))
						externalProcessTest.ExternalProcess.ParseConfig(configBuffer.data(), configBuffer.size());
					else
						externalProcessTest.WasConfigInvalid = true;
				}
				else
				{
					externalProcessTest.WasConfigInvalid = true;
				}
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

				Gui::BeginColumns("ProcessInfoColumns", 2, ImGuiOldColumnFlags_NoBorder);
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
				Gui::BeginColumns("CameraColumns", 2, ImGuiOldColumnFlags_NoBorder);
				{
					if (Gui::Checkbox("Sync Read Camera", &externalProcessTest.SyncReadCamera) && externalProcessTest.SyncReadCamera)
						activeViewport.CameraController.Mode = CameraController3D::ControlMode::None;
					Gui::NextColumn();
					if (Gui::Checkbox("Sync Write Camera", &externalProcessTest.SyncWriteCamera) && externalProcessTest.SyncWriteCamera)
						activeViewport.CameraController.Mode = CameraController3D::ControlMode::Orbit;
					Gui::NextColumn();
				}
				Gui::EndColumns();
				Gui::TreePop();
			}
			Gui::Separator();

			if (Gui::WideTreeNodeEx(ICON_FA_LIGHTBULB "  Light Param", ImGuiTreeNodeFlags_DefaultOpen))
			{
				Gui::BeginColumns("LightParamColumns", 2, ImGuiOldColumnFlags_NoBorder);
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
			activeViewport.Camera.ViewPoint = cameraData.ViewPoint;
			activeViewport.Camera.Interest = cameraData.Interest;
			activeViewport.Camera.FieldOfView = cameraData.FieldOfView;
		}
		else if (externalProcessTest.SyncWriteCamera && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteCamera({ activeViewport.Camera.ViewPoint, activeViewport.Camera.Interest, 0.0f, activeViewport.Camera.FieldOfView });
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

			if (scene.IBL != nullptr)
			{
				scene.IBL->Lights[0].LightColor = lightData.IBL0.LightColor;
				scene.IBL->Lights[0].IrradianceRGB = lightData.IBL0.IrradianceRGB;
				scene.IBL->Lights[1].LightColor = lightData.IBL1.LightColor;
				scene.IBL->Lights[1].IrradianceRGB = lightData.IBL1.IrradianceRGB;
			}
		}
		else if (externalProcessTest.SyncWriteLightParam && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteLightParam(
				{
					vec4(scene.Light.Character.Ambient, 1.0f),
					vec4(scene.Light.Character.Diffuse, 1.0f),
					scene.Light.Character.Specular,
					scene.Light.Character.Position,

					vec4(scene.Light.Stage.Ambient, 1.0f),
					vec4(scene.Light.Stage.Diffuse, 1.0f),
					scene.Light.Stage.Specular,
					scene.Light.Stage.Position,

					(scene.IBL == nullptr) ? vec4 {} : scene.IBL->Lights[0].LightColor,
					(scene.IBL == nullptr) ? std::array<mat4, 3> {} : scene.IBL->Lights[0].IrradianceRGB,
					(scene.IBL == nullptr) ? vec4 {} : scene.IBL->Lights[1].LightColor,
					(scene.IBL == nullptr) ? std::array<mat4, 3> {} : scene.IBL->Lights[1].IrradianceRGB,
				});
		}
	}

	void SceneEditor::DrawDebugTestGui(ViewportContext& activeViewport)
	{
#if 1 // DEBUG: Loaded textures image grid test
		static Gui::ImageGridView imageGridView;

		imageGridView.Begin();
		sceneGraph.TexIDMap.Iterate([&](ResourceIDMap<TexID, Tex>::ResourceIDPair& resourceIDPair)
		{
			TexID texID = resourceIDPair.ID;
			Tex& tex = *resourceIDPair.Resource;

			const auto guiID = GuiPropertyRAII::ID(&tex);
			imageGridView.Add(tex.GetName(), tex, tex.GetSize());
		});
		imageGridView.End();
#endif

#if 1
		static constexpr std::string_view effCmnObjSetPath = "dev_rom/objset/copy/effcmn/effcmn_obj.bin";
		static constexpr std::string_view effCmnTexSetPath = "dev_rom/objset/copy/effcmn/effcmn_tex.bin";

		static std::unique_ptr<ObjSet> effCmnObjSet = nullptr;

		if (effCmnObjSet == nullptr)
		{
			effCmnObjSet = IO::File::Load<ObjSet>(effCmnObjSetPath);
			effCmnObjSet->TexSet = TexSet::LoadSetTextureIDs(effCmnTexSetPath, effCmnObjSet.get());

			sceneGraph.RegisterTextures(effCmnObjSet->TexSet.get());

			if (auto sunObj = FindIfOrNull(effCmnObjSet->Objects, [&](const auto& obj) { return obj.Name == "effcmn_sun"; }); sunObj != nullptr)
			{
				scene.LensFlare.SunObj = sunObj;
				scene.LensFlare.Textures.Sun = sunObj->Materials.front().Textures.front().TextureID;
			}
		}

		for (const auto& objSet : sceneGraph.LoadedObjSets)
		{
			for (const auto& obj : objSet.ObjSet->Objects)
			{
				if (obj.Materials.empty())
					continue;

				if (Util::EndsWithInsensitive(obj.Name, "lensghost_0"))
					scene.LensFlare.Textures.Ghost.ID = obj.Materials.front().Textures[0].TextureID;
			}
		}

#endif
	}

	void SceneEditor::TakeScreenshotGui(ViewportContext& activeViewport)
	{
		const bool isScreenshotSaving = lastScreenshotTaskFuture.valid() && !lastScreenshotTaskFuture._Is_ready();
		const vec4 loadingColor = vec4(0.83f, 0.75f, 0.42f, 1.00f);

		if (isScreenshotSaving)
			Gui::PushStyleColor(ImGuiCol_Text, loadingColor);
		if (Gui::Button("Take Screenshot", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
			TakeSceneRenderTargetScreenshot(*activeViewport.RenderWindow->GetRenderTarget());
		if (isScreenshotSaving)
			Gui::PopStyleColor(1);
		Gui::ItemContextMenu("TakeScreenshotContextMenu", [&]
		{
			if (Gui::MenuItem("Open Directory"))
				IO::Shell::OpenInExplorer(ScreenshotDirectoy);
		});

		constexpr const char* renderPopupID = "RenderSequencePopup";

		if (Gui::Button("Render Sequence...", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
			Gui::OpenPopup(renderPopupID);

		if (Gui::BeginPopup(renderPopupID))
		{
			static struct SequenceData
			{
				i32 FramesToRender = 360 / 6;
				f32 RotationXStep = 6.0f;
				std::vector<std::future<void>> Futures;
			} data;

			Gui::InputInt("Frams To Render", &data.FramesToRender);
			Gui::InputFloat("Rotation Step", &data.RotationXStep);

			if (Gui::Button("Render!", vec2(Gui::GetContentRegionAvail().x, 0.0f)))
			{
				auto& activeRenderWindow = *activeViewport.RenderWindow;
				auto& renderTarget = *activeRenderWindow.GetRenderTarget();

				data.Futures.clear();
				data.Futures.reserve(data.FramesToRender);

				for (int i = 0; i < data.FramesToRender; i += 1)
				{
					activeViewport.CameraController.Mode = CameraController3D::ControlMode::Orbit;
					activeViewport.CameraController.OrbitData.TargetRotation.x = static_cast<f32>(i) * data.RotationXStep;
					activeViewport.CameraController.Update(activeViewport.Camera);

					activeRenderWindow.RenderScene();

					data.Futures.push_back(std::async(std::launch::async, [&renderTarget, i, data = std::move(renderTarget.TakeScreenshot())]
						{
							char fileName[MAX_PATH];
							sprintf_s(fileName, "%s/sequence/scene_%04d.png", ScreenshotDirectoy, i);
							Util::WriteImage(fileName, renderTarget.Param.RenderResolution, data.get());
						}));
				}
			}
			Gui::EndPopup();
		}
	}

	void SceneEditor::TakeSceneRenderTargetScreenshot(Render::RenderTarget3D& renderTarget)
	{
		auto pixelData = renderTarget.TakeScreenshot();

		if (lastScreenshotTaskFuture.valid())
			lastScreenshotTaskFuture.wait();

		lastScreenshotTaskFuture = std::async(std::launch::async, [&renderTarget, data = std::move(pixelData)]
			{
				const auto filePath = IO::Path::Combine(ScreenshotDirectoy, IO::Path::ChangeExtension("scene_" + FormatFileNameDateTimeNow(), ".png"));
				Util::WriteImage(filePath, renderTarget.Param.RenderResolution, data.get());
			});
	}
}

#include "SceneEditor.h"
#include "Core/ComfyData.h"
#include "Debug.h"
#include "Graphics/Auth3D/A3D.h"
#include "Graphics/Auth3D/A3DMgr.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "FileSystem/Archive/Farc.h"
#include "ImGui/Extensions/TxpExtensions.h"
#include "ImGui/Extensions/PropertyEditor.h"
#include "Misc/ImageHelper.h"
#include "Input/KeyCode.h"
#include <FontIcons.h>
#include <time.h>

namespace Comfy::Editor
{
	using namespace Graphics;

	constexpr const char* ScreenshotDirectoy = "dev_ram/ss";

	enum SceneEntityTag : EntityTag
	{
		NullTag = 0,
		StageTag = 'stg',
		CharacterTag = 'chr',
		ObjectTag = 'obj',
	};

	SceneEditor::SceneEditor(Application* parent, EditorManager* editor) : IEditorComponent(parent, editor)
	{
		auto txpGetter = [&](const Cached_TxpID* txpID) { return sceneGraph.TxpIDMap.Find(txpID); };
		renderer3D = MakeUnique<D3D_Renderer3D>(txpGetter);

		renderWindow = MakeUnique<SceneRenderWindow>(sceneGraph, viewport, scene, cameraController, *renderer3D);
	}

	void SceneEditor::Initialize()
	{
		renderWindow->Initialize();

		viewport.Camera.FieldOfView = 90.0f;

		viewport.Camera.ViewPoint = vec3(0.0f, 1.1f, 1.5f);
		viewport.Camera.Interest = vec3(0.0f, 1.0f, 0.0f);

		cameraController.FirstPersonData.TargetPitch = -11.0f;
		cameraController.FirstPersonData.TargetYaw = -90.000f;

		if (sceneGraph.LoadedObjSets.empty())
		{
			LoadStageObjects(StageType::STGTST, 7, 0);
			SetStageVisibility(StageVisibilityType::GroundSky);
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

		if (objSetPath == txpSetPath)
			return false;

		RefPtr<ObjSet> objSet = ObjSet::MakeUniqueReadParseUpload(objSetPath);
		objSet->Name = FileSystem::GetFileName(objSetPath, false);
		objSet->TxpSet = TxpSet::MakeUniqueReadParseUpload(txpSetPath, objSet.get());
		sceneGraph.LoadObjSet(objSet, tag);
		sceneGraph.RegisterTextures(objSet->TxpSet.get());

		if (sceneGraph.TxpDB == nullptr)
		{
			constexpr std::string_view txpDBPath = "dev_rom/db/tex_db.bin";
			sceneGraph.TxpDB = MakeUnique<Database::TxpDB>();

			if (FileSystem::FileExists(txpDBPath))
				sceneGraph.TxpDB->Load(std::string(txpDBPath));
		}

		if (sceneGraph.TxpDB != nullptr && objSet->TxpSet != nullptr)
		{
			for (auto& txp : objSet->TxpSet->Txps)
			{
				// TODO: Linear search yikesydoodles
				auto txpEntry = std::find_if(sceneGraph.TxpDB->Entries.begin(), sceneGraph.TxpDB->Entries.end(), [&](auto& e) { return e.ID == txp->ID; });
				if (txpEntry != sceneGraph.TxpDB->Entries.end())
					txp->Name.emplace(txpEntry->Name);
			}
		}

		return true;
	}

	bool SceneEditor::UnLoadUnRegisterObjSet(const ObjSet* objSetToRemove)
	{
		if (objSetToRemove == nullptr)
			return false;

		if (objSetToRemove->TxpSet != nullptr)
		{
			sceneGraph.TxpIDMap.RemoveIf([&](auto& pair)
			{
				return std::any_of(objSetToRemove->TxpSet->Txps.begin(), objSetToRemove->TxpSet->Txps.end(), [&pair](auto& txp) { return txp->ID == pair.ID; });
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
			auto checkTagUnregisterTxp = [&](ObjSetResource& objSetResource)
			{
				if (objSetResource.Tag == tag)
				{
					if (objSetResource.ObjSet->TxpSet != nullptr)
					{
						sceneGraph.TxpIDMap.RemoveIf([&](auto& pair)
						{
							return std::any_of(objSetResource.ObjSet->TxpSet->Txps.begin(), objSetResource.ObjSet->TxpSet->Txps.end(), [&pair](auto& txp) { return txp->ID == pair.ID; });
						});
					}
					return true;
				}
				return false;
			};

			sceneGraph.LoadedObjSets.erase(
				std::remove_if(sceneGraph.LoadedObjSets.begin(), sceneGraph.LoadedObjSets.end(),
					checkTagUnregisterTxp),
				sceneGraph.LoadedObjSets.end());
		}
	}

	void SceneEditor::DrawCameraGui()
	{
		auto& camera = viewport.Camera;

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
			cameraController.Visualization.InterestSphereObj = GenerateUploadDebugSphereObj(cameraController.Visualization.InterestSphere, cameraController.Visualization.InterestSphereColor);
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

			if (LoadRegisterObjSet(objSetFileViewer.GetFileToOpen(), Debug::GetTxpSetPathForObjSet(objSetFileViewer.GetFileToOpen()), ObjectTag))
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
		auto& renderParameters = viewport.Parameters;
		GuiPropertyRAII::ID id(&renderParameters);

		TakeScreenshotGui();

		GuiProperty::TreeNode("Render Parameters", ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_NoTreePushOnOpen, [&]
		{
			auto resolutionGui = [&](const char* label, const char* contextMenu, ivec2& inOutResolution)
			{
				auto clampValidTextureSize = [](ivec2 size) { return glm::clamp(size, GPU_Texture2D::MinSize, GPU_Texture2D::MaxSize); };

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
				GuiProperty::CheckboxFlags("DebugFlags_0", renderParameters.DebugFlags, (1 << 0));
				GuiProperty::CheckboxFlags("DebugFlags_1", renderParameters.DebugFlags, (1 << 1));
				GuiProperty::CheckboxFlags("ShaderDebugFlags_0", renderParameters.ShaderDebugFlags, (1 << 0));
				GuiProperty::CheckboxFlags("ShaderDebugFlags_1", renderParameters.ShaderDebugFlags, (1 << 1));
				GuiProperty::ColorEditHDR("ShaderDebugValue", renderParameters.ShaderDebugValue);
			});

			GuiProperty::TreeNode("General", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				resolutionGui("Main Scene", "RenderResolutionContextMenu", renderParameters.RenderResolution);
				resolutionGui("Reflection", "ReflectionResolutionContextMenu", renderParameters.ReflectionRenderResolution);

				if (GuiProperty::Input("Multi Sample Count", renderParameters.MultiSampleCount))
					renderParameters.MultiSampleCount = std::clamp(renderParameters.MultiSampleCount, 1u, 16u);

				if (GuiProperty::Input("Anistropic Filtering", renderParameters.AnistropicFiltering))
					renderParameters.AnistropicFiltering = std::clamp(renderParameters.AnistropicFiltering, D3D11_MIN_MAXANISOTROPY, D3D11_MAX_MAXANISOTROPY);

				GuiProperty::ColorEdit("Clear Color", renderParameters.ClearColor);
				GuiProperty::Checkbox("Clear", renderParameters.Clear);
				GuiProperty::Checkbox("Preserve Alpha", renderParameters.ToneMapPreserveAlpha);

				GuiProperty::Checkbox("Frustum Culling", renderParameters.FrustumCulling);
				GuiProperty::Checkbox("Alpha Sort", renderParameters.AlphaSort);
				GuiProperty::Checkbox("Wireframe", renderParameters.Wireframe);
			});

			GuiProperty::TreeNode("Shadow Mapping", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Shadow Mapping", renderParameters.ShadowMapping);
				GuiProperty::Checkbox("Self Shadowing", renderParameters.SelfShadowing);

				resolutionGui("Resolution", "ShadowMapResolutionContextMenu", renderParameters.ShadowMapResolution);

				if (GuiProperty::Input("Blur Passes", renderParameters.ShadowBlurPasses))
					renderParameters.ShadowBlurPasses = std::clamp(renderParameters.ShadowBlurPasses, 0u, 10u);
			});

			GuiProperty::TreeNode("Render Passes", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Reflection", renderParameters.RenderReflection);
				GuiProperty::Checkbox("Subsurface Scattering", renderParameters.RenderSubsurfaceScattering);
				GuiProperty::Checkbox("Opaque", renderParameters.RenderOpaque);
				GuiProperty::Checkbox("Transparent", renderParameters.RenderTransparent);
				GuiProperty::Checkbox("Bloom", renderParameters.RenderBloom);
				GuiProperty::Checkbox("Lens Flare", renderParameters.RenderLensFlare);
				GuiProperty::Checkbox("Auto Exposure", renderParameters.AutoExposure);
			});

			GuiProperty::TreeNode("Shader", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Vertex Coloring", renderParameters.VertexColoring);
				GuiProperty::Checkbox("Diffuse Mapping", renderParameters.DiffuseMapping);
				GuiProperty::Checkbox("Ambient Occlusion Mapping", renderParameters.AmbientOcclusionMapping);
				GuiProperty::Checkbox("Normal Mapping", renderParameters.NormalMapping);
				GuiProperty::Checkbox("Specular Mapping", renderParameters.SpecularMapping);
				GuiProperty::Checkbox("Transparency Mapping", renderParameters.TransparencyMapping);
				GuiProperty::Checkbox("Environment Mapping", renderParameters.EnvironmentMapping);
				GuiProperty::Checkbox("Translucency Mapping", renderParameters.TranslucencyMapping);
			});

			GuiProperty::TreeNode("Other", ImGuiTreeNodeFlags_DefaultOpen, [&]
			{
				GuiProperty::Checkbox("Punch Through", renderParameters.RenderPunchThrough);
				GuiProperty::Checkbox("Render Fog", renderParameters.RenderFog);
				GuiProperty::Checkbox("Object Morphing", renderParameters.ObjectMorphing);
				GuiProperty::Checkbox("Object Skinning", renderParameters.ObjectSkinning);
			});

		});

		GuiProperty::TreeNode("Render Targets", ImGuiTreeNodeFlags_NoTreePushOnOpen, [&]
		{
			auto renderTargetGui = [&](const char* name, auto& renderTarget, int index)
			{
				const uint32_t openMask = (1 << index);
				Gui::Selectable(name);

				if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(0))
					openRenderTargetsFlags ^= openMask;

				const float aspectRatio = (static_cast<float>(renderTarget.GetSize().y) / static_cast<float>(renderTarget.GetSize().x));
				if (openRenderTargetsFlags & openMask)
				{
					constexpr float desiredWidth = 512.0f;

					bool open = true;
					if (Gui::Begin(name, &open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoDocking))
						Gui::Image(renderTarget, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::End();
					if (!open)
						openRenderTargetsFlags &= ~openMask;
				}
				else if (Gui::IsItemHovered())
				{
					constexpr float desiredWidth = 256.0f;

					Gui::BeginTooltip();
					Gui::Image(renderTarget, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::EndTooltip();
				}
			};

			auto& renderData = viewport.Data;
			int index = 0;

			renderTargetGui("Main Current", renderData.Main.CurrentOrResolved(), index++);
			renderTargetGui("Main Previous", renderData.Main.PreviousOrResolved(), index++);

			renderTargetGui("Shadow Map", renderData.Shadow.RenderTarget, index++);

			renderTargetGui("Exponential Shadow Map [0]", renderData.Shadow.ExponentialRenderTargets[0], index++);
			renderTargetGui("Exponential Shadow Map [1]", renderData.Shadow.ExponentialRenderTargets[1], index++);
			renderTargetGui("Exponential Shadow Map Blur [0]", renderData.Shadow.ExponentialBlurRenderTargets[0], index++);
			renderTargetGui("Exponential Shadow Map Blur [1]", renderData.Shadow.ExponentialBlurRenderTargets[1], index++);

			renderTargetGui("Shadow Map Threshold", renderData.Shadow.ThresholdRenderTarget, index++);
			renderTargetGui("Shadow Map Blur [0]", renderData.Shadow.BlurRenderTargets[0], index++);
			renderTargetGui("Shadow Map Blur [1]", renderData.Shadow.BlurRenderTargets[1], index++);

			renderTargetGui("Screen Reflection", renderData.Reflection.RenderTarget, index++);

			renderTargetGui("SSS Main", renderData.SubsurfaceScattering.RenderTarget, index++);
			renderTargetGui("SSS Filter [0]", renderData.SubsurfaceScattering.FilterRenderTargets[0], index++);
			renderTargetGui("SSS Filter [1]", renderData.SubsurfaceScattering.FilterRenderTargets[1], index++);
			renderTargetGui("SSS Filter [2]", renderData.SubsurfaceScattering.FilterRenderTargets[2], index++);

			renderTargetGui("Bloom Base", renderData.Bloom.BaseRenderTarget, index++);
			renderTargetGui("Bloom Combined", renderData.Bloom.CombinedBlurRenderTarget, index++);
			renderTargetGui("Bloom Reduce->Blur [0]", renderData.Bloom.ReduceRenderTargets[0], index++);
			renderTargetGui("Bloom Reduce->Blur [1]", renderData.Bloom.ReduceRenderTargets[1], index++);
			renderTargetGui("Bloom Reduce->Blur [2]", renderData.Bloom.ReduceRenderTargets[2], index++);
			renderTargetGui("Bloom Reduce->Blur [3]", renderData.Bloom.ReduceRenderTargets[3], index++);

			renderTargetGui("Exposure [0]", renderData.Bloom.ExposureRenderTargets[0], index++);
			renderTargetGui("Exposure [1]", renderData.Bloom.ExposureRenderTargets[1], index++);
			renderTargetGui("Exposure [2]", renderData.Bloom.ExposureRenderTargets[2], index++);

			renderTargetGui("Output", renderData.Output.RenderTarget, index++);
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
					if (lightMap.GPU_CubeMap != nullptr)
					{
						constexpr vec2 cubeMapDisplaySize = vec2(96.0f, 96.0f);

						const float width = std::clamp(Gui::GetContentRegionAvailWidth(), 1.0f, cubeMapDisplaySize.x);
						const vec2 size = vec2(width, width * (3.0f / 4.0f));

						ImTextureID textureID = *lightMap.GPU_CubeMap;
						textureID.Data.CubeMapMipLevel = 0;
						Gui::Image(textureID, size);
						Gui::SameLine();
						textureID.Data.CubeMapMipLevel = 1;
						Gui::Image(textureID, size);
					}
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

			viewport.Camera.Interest = viewport.Camera.ViewPoint = transformedSphere.Center;
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

		GuiProperty::TreeNode("Material Editor", [&]
		{
			if (!collectionComboDebugMaterialHover("Material", selectedObj->Materials, objTestData.MaterialIndex))
				return;

			Material& material = selectedObj->Materials[objTestData.MaterialIndex];
			materialEditor.DrawGui(*renderer3D, scene, material);
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
				auto[objPathType, txpPathType] = (isCommonItem) ?
					std::make_pair(Debug::PathType::CmnItemObj, Debug::PathType::CmnItemTxp) :
					std::make_pair(Debug::PathType::CharaItemObj, Debug::PathType::CharaItemTxp);

				auto objSetPath = Debug::GetDebugFilePath(objPathType, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());
				auto txpSetPath = Debug::GetDebugFilePath(txpPathType, StageType::STGTST, id, 0, charaTestData.IDs.Character.data());

				if (LoadRegisterObjSet(objSetPath.data(), txpSetPath.data(), CharacterTag))
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
		// TODO:
	}

	void SceneEditor::DrawExternalProcessTestGui()
	{
		auto tryAttach = [&]
		{
			if (externalProcessTest.ShouldReadConfigFile)
			{
				externalProcessTest.ShouldReadConfigFile = false;

				std::vector<uint8_t> fileBuffer;
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
			viewport.Camera.ViewPoint = cameraData.ViewPoint;
			viewport.Camera.Interest = cameraData.Interest;
			viewport.Camera.FieldOfView = cameraData.FieldOfView;
		}
		else if (externalProcessTest.SyncWriteCamera && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteCamera({ viewport.Camera.ViewPoint, viewport.Camera.Interest, 0.0f, viewport.Camera.FieldOfView });
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
			sceneGraph.TxpIDMap.Iterate([&](ResourceIDMap<TxpID, Txp>::ResourceIDPair& resourceIDPair)
			{
				TxpID id = resourceIDPair.ID;
				Txp& txp = *resourceIDPair.Resource;

				char buffer[64];
				sprintf_s(buffer, "0x%X : %s", id, txp.GetName().data());
				Gui::Selectable(buffer, false);
				if (Gui::IsItemHovered())
				{
					Gui::BeginTooltip();
					Gui::ImageObjTxp(&txp);
					Gui::EndTooltip();
				}
			});
		});
#endif

		static struct DebugData
		{
			static const A3DObject* FindA3DObjectParent(const A3D& a3d, const A3DObject& object)
			{
				if (object.ParentName.empty())
					return nullptr;

				// TODO: Search object name list instead (?)
				auto parent = std::find_if(a3d.Objects.begin(), a3d.Objects.end(), [&](auto& o) { return o.Name == object.ParentName; });
				return (parent == a3d.Objects.end()) ? nullptr : &(*parent);
			}

			static void ApplyA3DParentTransform(const A3D& a3d, const A3DObject& parentObject, Transform& output, frame_t frame)
			{
				auto& a3dObjects = a3d.Objects;

				Transform parentTransform = A3DMgr::GetTransformAt(parentObject.Transform, frame);

				if (auto nestedParent = FindA3DObjectParent(a3d, parentObject); nestedParent != nullptr)
					ApplyA3DParentTransform(a3d, *nestedParent, parentTransform, frame);

				output.ApplyParent(parentTransform);
			}

			float MorphWeight = 0.0f, PlaybackSpeed = 0.0f, Elapsed = 0.0f;

			std::vector<A3D> StageEffA3Ds, CamPVA3Ds;
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

				if (entity->Animation == nullptr)
					entity->Animation = MakeUnique<ObjAnimationData>();

				entity->MorphObj = nextEntity->Obj;
				entity->Animation->MorphWeight = debug.MorphWeight;

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
				if (entity->MorphObj == nullptr)
					continue;

				if (entity->Obj->Meshes.size() != entity->MorphObj->Meshes.size())
					entity->MorphObj = nullptr;
			}
		}

		if (Gui::CollapsingHeader("A3D Test", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto loadA3Ds = [](const char* farcPath)
			{
				std::vector<A3D> a3ds;
				if (auto farc = FileSystem::Farc::Open(farcPath); farc != nullptr)
				{
					for (auto& file : *farc)
					{
						auto content = file.ReadVector();
						a3ds.emplace_back().Parse(content.data(), content.size());
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
					//auto correspondingEntity = std::find_if(entities.begin(), entities.end(), [&](auto& e) { return e->Name == object.UIDName; });

					if (correspondingEntity == entities.end())
						continue;

					auto& entity = (*correspondingEntity);

					entity->Transform = A3DMgr::GetTransformAt(object.Transform, frame);
					entity->IsVisible = A3DMgr::GetBoolAt(object.Transform.Visibility, frame);

					if (auto parent = DebugData::FindA3DObjectParent(a3d, object); parent != nullptr)
						DebugData::ApplyA3DParentTransform(a3d, *parent, entity->Transform, frame);

					if (entity->Animation == nullptr)
						entity->Animation = MakeUnique<ObjAnimationData>();

					auto findCurve = [&](auto& name) -> const A3DCurve*
					{
						if (name.empty())
							return nullptr;

						auto found = std::find_if(a3d.Curves.begin(), a3d.Curves.end(), [&](auto& curve) { return MatchesInsensitive(curve.Name, name); });
						//auto found = std::find_if(a3d.Curves.begin(), a3d.Curves.end(), [&](auto& curve) { return curve.Name == name; });
						return (found == a3d.Curves.end()) ? nullptr : &(*found);
					};

					// TODO: Instead of searching at the entire TxpDB for entries only the loaded ObjSets would have to be checked (?)
					//		 As long as their Txps have been updated using a TxpDB before that is

					if (!object.TexturePatterns.empty())
					{
						auto& a3dPatterns = object.TexturePatterns;
						auto& entityPatterns = entity->Animation->TexturePatterns;

						if (entityPatterns.size() != a3dPatterns.size())
							entityPatterns.resize(a3dPatterns.size());

						for (size_t i = 0; i < a3dPatterns.size(); i++)
						{
							auto& a3dPattern = a3dPatterns[i];
							auto& entityPattern = entityPatterns[i];

							if (auto pattern = findCurve(a3dPattern.Pattern); pattern != nullptr)
							{
								if (!entityPattern.CachedIDs.has_value())
								{
									auto& cachedIDs = entityPattern.CachedIDs.emplace();
									cachedIDs.reserve(16);

									for (int cacheIndex = 0; cacheIndex < 999; cacheIndex++)
									{
										char nameBuffer[128];
										sprintf_s(nameBuffer, "%.*s_%03d", static_cast<int>(a3dPattern.Name.size() - strlen("_000")), a3dPattern.Name.data(), cacheIndex);

										auto txpEntry = sceneGraph.TxpDB->GetTxpEntry(nameBuffer);
										if (txpEntry == nullptr)
											break;

										if (cacheIndex == 0)
											entityPattern.ID = txpEntry->ID;

										cachedIDs.push_back(txpEntry->ID);
									}
								}

								const int index = A3DMgr::GetIntAt(pattern->CV, frame);
								entityPattern.IDOverride = (InBounds(index, *entityPattern.CachedIDs)) ? entityPattern.CachedIDs->at(index) : TxpID::Invalid;
							}
						}
					}

					if (!object.TextureTransforms.empty())
					{
						auto& a3dTexTransforms = object.TextureTransforms;
						auto& entityTexTransforms = entity->Animation->TextureTransforms;

						if (entityTexTransforms.size() != a3dTexTransforms.size())
							entityTexTransforms.resize(a3dTexTransforms.size());

						for (size_t i = 0; i < a3dTexTransforms.size(); i++)
						{
							auto& a3dTexTransform = a3dTexTransforms[i];
							auto& entityTexTransform = entityTexTransforms[i];

							if (entityTexTransform.ID == TxpID::Invalid)
							{
								if (auto txpEntry = sceneGraph.TxpDB->GetTxpEntry(a3dTexTransform.Name); txpEntry != nullptr)
									entityTexTransform.ID = txpEntry->ID;
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

					if (auto morphCurve = findCurve(object.Morph); morphCurve != nullptr)
					{
						size_t morphEntityIndex = static_cast<size_t>(std::distance(entities.begin(), correspondingEntity)) + 1;

						entity->Animation->MorphWeight = A3DMgr::GetValueAt(morphCurve->CV, frame);
						entity->MorphObj = (morphEntityIndex >= entities.size()) ? nullptr : entities[morphEntityIndex]->Obj;
					}
				}

				for (auto& a3dCamera : a3d.CameraRoot)
				{
					camera.ViewPoint = A3DMgr::GetValueAt(a3dCamera.ViewPoint.Transform.Translation, frame);
					camera.Interest = A3DMgr::GetValueAt(a3dCamera.Interest.Translation, frame);
					camera.FieldOfView = A3DMgr::GetFieldOfViewAt(a3dCamera.ViewPoint, frame);
				}
			};

			auto iterateA3Ds = [](std::vector<A3D>& a3ds, int index, auto func)
			{
				if (a3ds.empty())
					return;
				else if (index < 0)
					for (auto& a3d : a3ds)
						func(a3d);
				else if (index < a3ds.size())
					func(a3ds[index]);
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

			if (Gui::IsWindowFocused() && Gui::IsKeyPressed(KeyCode_Space))
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
				iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, viewport.Camera); });

			if (cameraController.Mode == CameraController3D::ControlMode::None)
				iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, scene, viewport.Camera); });

			if (Gui::Button("Camera Mode Orbit")) { viewport.Camera.FieldOfView = 90.0f; cameraController.Mode = CameraController3D::ControlMode::Orbit; }
			Gui::SameLine();
			if (Gui::Button("Camera Mode None")) { cameraController.Mode = CameraController3D::ControlMode::None; }

			if (Gui::Button("Set Screen Render IDs"))
			{
				for (auto& entity : sceneGraph.Entities)
				{
					for (const auto& material : entity->Obj->Materials)
					{
						auto diffuseTexture = std::find_if(material.Textures.begin(), material.Textures.end(), [](auto& texture) { return texture.TextureFlags.Type == MaterialTextureType::ColorMap; });
						if (diffuseTexture != material.Textures.end())
						{
							if (auto txp = renderer3D->GetTxpFromTextureID(&diffuseTexture->TextureID); txp != nullptr && txp->Name.has_value())
							{
								if (auto& name = txp->Name.value();
									EndsWithInsensitive(name, "_RENDER") ||
									EndsWithInsensitive(name, "_MOVIE") ||
									EndsWithInsensitive(name, "_TV") ||
									EndsWithInsensitive(name, "_FB01") ||
									EndsWithInsensitive(name, "_FB02") ||
									EndsWithInsensitive(name, "_FB03"))
								{
									if (entity->Animation == nullptr)
										entity->Animation = MakeUnique<ObjAnimationData>();

									entity->Animation->ScreenRenderTextureID = diffuseTexture->TextureID;
								}
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
		if (Gui::Button("Take Screenshot", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
			TakeSceneRenderTargetScreenshot(viewport.Data.Output.RenderTarget);
		if (isScreenshotSaving)
			Gui::PopStyleColor(1);
		Gui::ItemContextMenu("TakeScreenshotContextMenu", [&]
		{
			if (Gui::MenuItem("Open Directory"))
				FileSystem::OpenInExplorer(Utf8ToUtf16(ScreenshotDirectoy));
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

			if (Gui::Button("Render!", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
			{
				auto& renderTarget = viewport.Data.Output.RenderTarget;

				data.Futures.clear();
				data.Futures.reserve(data.FramesToRender);

				for (int i = 0; i < data.FramesToRender; i += 1)
				{
					cameraController.Mode = CameraController3D::ControlMode::Orbit;
					cameraController.OrbitData.TargetRotation.x = static_cast<float>(i) * data.RotationXStep;
					cameraController.Update(viewport.Camera);

					renderWindow->RenderScene();

					data.Futures.push_back(std::async(std::launch::async, [&renderTarget, i, data = std::move(renderTarget.StageAndCopyBackBuffer())]
						{
							char fileName[MAX_PATH];
							sprintf_s(fileName, "%s/sequence/scene_%04d.png", ScreenshotDirectoy, i);
							Utilities::WritePNG(fileName, renderTarget.GetSize(), data.get());
						}));
				}
			}
			Gui::EndPopup();
		}
	}

	void SceneEditor::TakeSceneRenderTargetScreenshot(GPU_RenderTarget& renderTarget)
	{
		auto pixelData = renderTarget.StageAndCopyBackBuffer();

		if (lastScreenshotTaskFuture.valid())
			lastScreenshotTaskFuture.wait();

		lastScreenshotTaskFuture = std::async(std::launch::async, [&renderTarget, data = std::move(pixelData)]
			{
				char fileName[MAX_PATH];
				sprintf_s(fileName, "%s/scene_%I64d.png", ScreenshotDirectoy, static_cast<int64_t>(time(nullptr)));

				Utilities::WritePNG(fileName, renderTarget.GetSize(), data.get());
			});
	}
}

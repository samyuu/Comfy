#include "SceneEditor.h"
#include "Core/ComfyData.h"
#include "Debug.h"
#include "Graphics/Auth3D/A3D.h"
#include "Graphics/Auth3D/A3DMgr.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "FileSystem/Archive/Farc.h"
#include "ImGui/Extensions/TxpExtensions.h"
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

		renderWindow = MakeUnique<SceneRenderWindow>(sceneGraph, context, cameraController, *renderer3D);
	}

	void SceneEditor::Initialize()
	{
		renderWindow->Initialize();

		context.Camera.FieldOfView = 90.0f;

		context.Camera.ViewPoint = vec3(0.0f, 1.1f, 1.5f);
		context.Camera.Interest = vec3(0.0f, 1.0f, 0.0f);

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
		{
			static Transform objTrans = Transform(vec3(0.0f));
			if (Gui::DragFloat3("Position", glm::value_ptr(objTrans.Translation), 0.1f) |
				Gui::DragFloat3("Scale", glm::value_ptr(objTrans.Scale), 0.1f) |
				Gui::DragFloat3("Rotation", glm::value_ptr(objTrans.Rotation), 0.1f))
				for (auto& entity : sceneGraph.Entities)
					if (entity->Tag == ObjectTag) entity->Transform = objTrans;

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
				return std::any_of(objSetToRemove->TxpSet->Txps.begin(), objSetToRemove->TxpSet->Txps.end(), [&pair](auto& txp)  { return txp->ID == pair.ID; });
			});
		}

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
				e->IsVisible = (e->Name.find("_gnd") != std::string::npos || e->Name.find("_sky") != std::string::npos) || (e->Obj != nullptr && Debug::IsReflectionObj(*e->Obj));
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
		auto& camera = context.Camera;

		Gui::PushID(&camera);
		Gui::DragFloat("Field Of View", &camera.FieldOfView, 1.0f, 1.0f, 173.0f);
		Gui::DragFloat("Near Plane", &camera.NearPlane, 0.001f, 0.001f, 1.0f);
		Gui::DragFloat("Far Plane", &camera.FarPlane);
		Gui::DragFloat3("View Point", glm::value_ptr(camera.ViewPoint), 0.01f);
		Gui::DragFloat3("Interest", glm::value_ptr(camera.Interest), 0.01f);
		Gui::DragFloat("Smoothness", &cameraController.Settings.InterpolationSmoothness, 1.0f, 0.0f, 250.0f);

		Gui::Combo("Control Mode", reinterpret_cast<int*>(&cameraController.Mode), "None\0First Person\0Orbit\0");

		Gui::ItemContextMenu("ControlModeContextMenu", [&]()
		{
			Gui::Text("Camera Presets:");
			Gui::Separator();
			if (Gui::MenuItem("Orbit Default"))
			{
				cameraController.Mode = CameraController3D::ControlMode::Orbit;
				cameraController.OrbitData.Distance = 5.0f;
				cameraController.OrbitData.TargetRotation = vec3(0.0f);
				context.Camera.FieldOfView = 90.0f;
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

		const bool isScreenshotSaving = lastScreenshotTaskFuture.valid() && !lastScreenshotTaskFuture._Is_ready();
		const vec4 loadingColor = vec4(0.83f, 0.75f, 0.42f, 1.00f);

		if (isScreenshotSaving)
			Gui::PushStyleColor(ImGuiCol_Text, loadingColor);
		if (Gui::Button("Take Screenshot", vec2(Gui::GetContentRegionAvailWidth(), 0.0f)))
			TakeSceneRenderTargetScreenshot(context.RenderData.Output.RenderTarget);
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
				auto& renderTarget = context.RenderData.Output.RenderTarget;

				data.Futures.clear();
				data.Futures.reserve(data.FramesToRender);

				for (int i = 0; i < data.FramesToRender; i += 1)
				{
					cameraController.Mode = CameraController3D::ControlMode::Orbit;
					cameraController.OrbitData.TargetRotation.x = static_cast<float>(i) * data.RotationXStep;
					cameraController.Update(context.Camera);

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

		Gui::CheckboxFlags("DebugFlags_0", &renderParameters.DebugFlags, (1 << 0)); Gui::SameLine(); Gui::CheckboxFlags("ShaderDebugFlags_0", &renderParameters.ShaderDebugFlags, (1 << 0));
		Gui::CheckboxFlags("DebugFlags_1", &renderParameters.DebugFlags, (1 << 1)); Gui::SameLine(); Gui::CheckboxFlags("ShaderDebugFlags_1", &renderParameters.ShaderDebugFlags, (1 << 1));
		Gui::ColorEdit4("ShaderDebugValue", glm::value_ptr(renderParameters.ShaderDebugValue));
		Gui::Separator();
		Gui::ColorEdit4("Clear Color", glm::value_ptr(renderParameters.ClearColor));
		Gui::Checkbox("Clear", &renderParameters.Clear);
		Gui::Checkbox("Preserve Alpha", &renderParameters.ToneMapPreserveAlpha);
		Gui::Separator();
		Gui::Checkbox("Frustum Culling", &renderParameters.FrustumCulling);
		Gui::Checkbox("Wireframe", &renderParameters.Wireframe);
		Gui::Checkbox("Alpha Sort", &renderParameters.AlphaSort);
		Gui::Separator();
		Gui::Checkbox("Shadow Mapping", &renderParameters.ShadowMapping);
		Gui::Checkbox("Self Shadowing", &renderParameters.SelfShadowing);
		Gui::Separator();
		Gui::Checkbox("Render Reflection", &renderParameters.RenderReflection);
		Gui::Checkbox("Render Subsurface Scattering", &renderParameters.RenderSubsurfaceScattering);
		Gui::Checkbox("Render Opaque", &renderParameters.RenderOpaque);
		Gui::Checkbox("Render Transparent", &renderParameters.RenderTransparent);
		Gui::Checkbox("Render Bloom", &renderParameters.RenderBloom);
		Gui::Checkbox("Auto Exposure", &renderParameters.AutoExposure);
		Gui::Checkbox("Vertex Coloring", &renderParameters.VertexColoring);
		Gui::Separator();
		Gui::Checkbox("Diffuse Mapping", &renderParameters.DiffuseMapping);
		Gui::Checkbox("Ambient Occlusion Mapping", &renderParameters.AmbientOcclusionMapping);
		Gui::Checkbox("Normal Mapping", &renderParameters.NormalMapping);
		Gui::Checkbox("Specular Mapping", &renderParameters.SpecularMapping);
		Gui::Checkbox("Transparency Mapping", &renderParameters.TransparencyMapping);
		Gui::Checkbox("Environment Mapping", &renderParameters.EnvironmentMapping);
		Gui::Checkbox("Translucency Mapping", &renderParameters.TranslucencyMapping);
		Gui::Separator();
		Gui::Checkbox("Render Punch Through", &renderParameters.RenderPunchThrough);
		Gui::Checkbox("Render Fog", &renderParameters.RenderFog);
		Gui::Separator();
		Gui::SliderInt("Anistropic Filtering", &renderParameters.AnistropicFiltering, D3D11_MIN_MAXANISOTROPY, D3D11_MAX_MAXANISOTROPY);

		if (Gui::CollapsingHeader("Resolution", ImGuiTreeNodeFlags_DefaultOpen))
		{
			auto clampSize = [](ivec2 size) { return glm::clamp(size, D3D_Texture2D::MinSize, D3D_Texture2D::MaxSize); };

			constexpr std::array namedFactors =
			{
				std::make_pair("Render Region x0.5", 0.5f),
				std::make_pair("Render Region x1.0", 1.0f),
				std::make_pair("Render Region x2.0", 2.0f),
				std::make_pair("Render Region x4.0", 4.0f),
				std::make_pair("Render Region x8.0", 8.0f),
				std::make_pair("Render Region x16.0", 16.0f),
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

			if (renderResolution != context.RenderData.Main.Current().GetSize())
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

			ivec2 shadowResolution = renderParameters.ShadowMapResolution;
			Gui::InputInt2("Shadow Resolution", glm::value_ptr(shadowResolution));
			Gui::ItemContextMenu("ShadowResolutionContextMenu", [&]()
			{
				Gui::Text("Set Shadow Resolution:");
				Gui::Separator();
				if (Gui::MenuItem("256x256")) shadowResolution = ivec2(256, 256);
				if (Gui::MenuItem("512x512")) shadowResolution = ivec2(512, 512);
				if (Gui::MenuItem("1024x1024")) shadowResolution = ivec2(1024, 1024);
				if (Gui::MenuItem("2048x2048")) shadowResolution = ivec2(2048, 2048);
				if (Gui::MenuItem("4096x4096")) shadowResolution = ivec2(4096, 4096);
				if (Gui::MenuItem("8192x8192")) shadowResolution = ivec2(8192, 8192);
			});

			if (shadowResolution != context.RenderData.Shadow.RenderTarget.GetSize())
				renderParameters.ShadowMapResolution = (clampSize(shadowResolution));

			if (Gui::InputScalar("Shadow Blur Passes", ImGuiDataType_U32, &renderParameters.ShadowBlurPasses))
				renderParameters.ShadowBlurPasses = std::clamp(renderParameters.ShadowBlurPasses, 0u, 10u);

			if (Gui::InputScalar("Multi Sample Count", ImGuiDataType_U32, &renderParameters.MultiSampleCount))
				renderParameters.MultiSampleCount = std::clamp(renderParameters.MultiSampleCount, 1u, 16u);
		}

		if (Gui::CollapsingHeader("Render Targets"))
		{
			auto& renderData = context.RenderData;

			static uint32_t openFlags = 0;
			static uint32_t currentIndex = 0;

			auto renderTargetGui = [&](const char* name, auto& renderTarget)
			{
				const uint32_t openMask = (1 << currentIndex);
				currentIndex++;

				Gui::Selectable(name);

				if (Gui::IsItemHovered() && Gui::IsMouseDoubleClicked(0))
					openFlags ^= openMask;

				const float aspectRatio = (static_cast<float>(renderTarget.GetSize().y) / static_cast<float>(renderTarget.GetSize().x));
				if (openFlags & openMask)
				{
					constexpr float desiredWidth = 512.0f;

					bool open = true;
					if (Gui::Begin(name, &open, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoDocking))
						Gui::Image(renderTarget, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::End();
					if (!open)
						openFlags &= ~openMask;
				}
				else if (Gui::IsItemHovered())
				{
					constexpr float desiredWidth = 256.0f;

					Gui::BeginTooltip();
					Gui::Image(renderTarget, vec2(desiredWidth, desiredWidth * aspectRatio));
					Gui::EndTooltip();
				}
			};

			currentIndex = 0;
			renderTargetGui("Main Current", renderData.Main.CurrentOrResolved());
			renderTargetGui("Main Previous", renderData.Main.PreviousOrResolved());

			renderTargetGui("Shadow Map", renderData.Shadow.RenderTarget);

			renderTargetGui("Exponential Shadow Map [0]", renderData.Shadow.ExponentialRenderTargets[0]);
			renderTargetGui("Exponential Shadow Map [1]", renderData.Shadow.ExponentialRenderTargets[1]);
			renderTargetGui("Exponential Shadow Map Blur [0]", renderData.Shadow.ExponentialBlurRenderTargets[0]);
			renderTargetGui("Exponential Shadow Map Blur [1]", renderData.Shadow.ExponentialBlurRenderTargets[1]);

			renderTargetGui("Shadow Map Threshold", renderData.Shadow.ThresholdRenderTarget);
			renderTargetGui("Shadow Map Blur [0]", renderData.Shadow.BlurRenderTargets[0]);
			renderTargetGui("Shadow Map Blur [1]", renderData.Shadow.BlurRenderTargets[1]);

			renderTargetGui("Screen Reflection", renderData.Reflection.RenderTarget);

			renderTargetGui("SSS Main", renderData.SubsurfaceScattering.RenderTarget);
			renderTargetGui("SSS Filter [0]", renderData.SubsurfaceScattering.FilterRenderTargets[0]);
			renderTargetGui("SSS Filter [1]", renderData.SubsurfaceScattering.FilterRenderTargets[1]);
			renderTargetGui("SSS Filter [2]", renderData.SubsurfaceScattering.FilterRenderTargets[2]);

			renderTargetGui("Bloom Base", renderData.Bloom.BaseRenderTarget);
			renderTargetGui("Bloom Combined", renderData.Bloom.CombinedBlurRenderTarget);
			renderTargetGui("Bloom Reduce->Blur [0]", renderData.Bloom.ReduceRenderTargets[0]);
			renderTargetGui("Bloom Reduce->Blur [1]", renderData.Bloom.ReduceRenderTargets[1]);
			renderTargetGui("Bloom Reduce->Blur [2]", renderData.Bloom.ReduceRenderTargets[2]);
			renderTargetGui("Bloom Reduce->Blur [3]", renderData.Bloom.ReduceRenderTargets[3]);

			renderTargetGui("Exposure [0]", renderData.Bloom.ExposureRenderTargets[0]);
			renderTargetGui("Exposure [1]", renderData.Bloom.ExposureRenderTargets[1]);
			renderTargetGui("Exposure [2]", renderData.Bloom.ExposureRenderTargets[2]);

			renderTargetGui("Output", renderData.Output.RenderTarget);
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

		Gui::Checkbox("Auto Exposure", &context.Glow.AutoExposure);
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
				Gui::TreePop();
			}
			Gui::PopID();
		};

		auto iblLightMapGui = [](const char* name, LightMap& lightMap, ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_None)
		{
			Gui::PushID(&lightMap);
			if (Gui::WideTreeNodeEx(name, flags))
			{
				constexpr float cubeMapSize = 60.0f;
				if (lightMap.D3D_CubeMap != nullptr)
					Gui::ImageButton(*lightMap.D3D_CubeMap, vec2(cubeMapSize, cubeMapSize * (3.0f / 4.0f)));

				Gui::TreePop();
			}
			Gui::PopID();
		};

		Gui::PushID(&context.IBL);

		static std::array<char, MAX_PATH> iblPathBuffer = { "dev_rom/ibl/tst.ibl" };
		if (Gui::InputText("Load IBL", iblPathBuffer.data(), iblPathBuffer.size(), ImGuiInputTextFlags_EnterReturnsTrue))
			Debug::LoadParseUploadLightParamFile(iblPathBuffer.data(), context.IBL);

		iblLightDataGui("Character", context.IBL.Character, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Stage", context.IBL.Stage, ImGuiTreeNodeFlags_DefaultOpen);
		iblLightDataGui("Sun", context.IBL.Sun, ImGuiTreeNodeFlags_DefaultOpen);

		for (size_t i = 0; i < context.IBL.LightMaps.size(); i++)
		{
			char buffer[64];
			sprintf_s(buffer, "LightMaps[%zu]", i);
			iblLightMapGui(buffer, context.IBL.LightMaps[i], ImGuiTreeNodeFlags_DefaultOpen);
		}

		Gui::PopID();
	}

	void SceneEditor::DrawSceneGraphGui()
	{
		Gui::BeginChild("SceneGraphEntiriesListChild");

		auto setCamera = [&](auto& entity, const Sphere& boundingSphere)
		{
			const Sphere transformedSphere = boundingSphere * entity->Transform;

			context.Camera.Interest = context.Camera.ViewPoint = transformedSphere.Center;
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
				if (Gui::InputText("Material Type", material->MaterialType.data(), material->MaterialType.size(), ImGuiInputTextFlags_None))
					std::fill(std::find(material->MaterialType.begin(), material->MaterialType.end(), '\0'), material->MaterialType.end(), '\0');

				bool lambertShading = material->ShaderFlags.LambertShading;
				if (Gui::Checkbox("Lambert Shading", &lambertShading))
					material->ShaderFlags.LambertShading = lambertShading;

				bool phongShading = material->ShaderFlags.PhongShading;
				if (Gui::Checkbox("Phong Shading", &phongShading))
					material->ShaderFlags.PhongShading = phongShading;

				for (size_t i = 0; i < material->TexturesArray.size(); i++)
				{
					if (auto txp = renderer3D->GetTxpFromTextureID(&material->TexturesArray[i].TextureID); txp != nullptr)
					{
						Gui::ImageObjTxp(txp, vec2(120.0f));

						if (Gui::IsItemHovered())
							Gui::SetTooltip("%s: %s", MaterialTexture::TextureTypeNames[i], txp->GetName().data());
					}
				}
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
				bool faceCameraPosition = mesh->Flags.FaceCameraPosition;
				if (Gui::Checkbox("Face Camera Position", &faceCameraPosition))
					mesh->Flags.FaceCameraPosition = faceCameraPosition;

				bool faceCameraView = mesh->Flags.FaceCameraView;
				if (Gui::Checkbox("Face Camera View", &faceCameraView))
					mesh->Flags.FaceCameraView = faceCameraView;

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
					SetStageVisibility(StageVisibilityType::GroundSky);
				}

				stageTestData.lastSetStage.emplace(stageTypeData);
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
		Gui::Checkbox("Load Stage Obj", &stageTestData.Settings.LoadObj);

		const float availWidth = Gui::GetContentRegionAvailWidth();

		if (Gui::Button("Show All", vec2(availWidth * 0.8f, 0.0f)))
			SetStageVisibility(StageVisibilityType::All);

		if (Gui::Button("Hide All", vec2(availWidth * 0.8f, 0.0f)))
			SetStageVisibility(StageVisibilityType::None);

		if (Gui::Button("Show Ground & Sky", vec2(availWidth * 0.8f, 0.0f)))
			SetStageVisibility(StageVisibilityType::GroundSky);
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
			loadPart(charaTestData.IDs.CommonItem, -1, true);
			loadPart(charaTestData.IDs.Face, charaTestData.IDs.FaceIndex);
			loadPart(charaTestData.IDs.Overhead);
			loadPart(charaTestData.IDs.Hair);
			loadPart(charaTestData.IDs.Outer);
			loadPart(charaTestData.IDs.Hands);
		};

		Gui::PushID(&charaTestData);
		Gui::InputText("Character", charaTestData.IDs.Character.data(), charaTestData.IDs.Character.size());

		if (Gui::InputInt("Item", &charaTestData.IDs.CommonItem))
			loadCharaItems();

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
		bool scaleChanged = Gui::DragFloat3("Scale", glm::value_ptr(charaTestData.Transform.Scale), 0.1f);
		bool rotChanged = Gui::DragFloat3("Rotation", glm::value_ptr(charaTestData.Transform.Rotation), 0.1f);

		if (posChanged | scaleChanged | rotChanged)
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
			context.Camera.ViewPoint = cameraData.ViewPoint;
			context.Camera.Interest = cameraData.Interest;
			context.Camera.FieldOfView = cameraData.FieldOfView;
		}
		else if (externalProcessTest.SyncWriteCamera && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteCamera({ context.Camera.ViewPoint, context.Camera.Interest, 0.0f, context.Camera.FieldOfView });
		}

		if (externalProcessTest.SyncReadLightParam && tryAttach())
		{
			const auto lightData = externalProcessTest.ExternalProcess.ReadLightParam();

			context.Light.Character.Ambient = lightData.Character.Ambient;
			context.Light.Character.Diffuse = lightData.Character.Diffuse;
			context.Light.Character.Specular = lightData.Character.Specular;
			context.Light.Character.Position = lightData.Character.Position;

			context.Light.Stage.Ambient = lightData.Stage.Ambient;
			context.Light.Stage.Diffuse = lightData.Stage.Diffuse;
			context.Light.Stage.Specular = lightData.Stage.Specular;
			context.Light.Stage.Position = lightData.Stage.Position;

			context.IBL.Character.LightColor = lightData.IBLCharacter.Color;
			context.IBL.Character.IrradianceRGB = lightData.IBLCharacter.Matrices;
			context.IBL.Stage.LightColor = lightData.IBLStage.Color;
			context.IBL.Stage.IrradianceRGB = lightData.IBLStage.Matrices;
		}
		else if (externalProcessTest.SyncWriteLightParam && tryAttach())
		{
			externalProcessTest.ExternalProcess.WriteLightParam(
				{
					vec4(context.Light.Character.Ambient, 1.0f),
					vec4(context.Light.Character.Diffuse, 1.0f),
					vec4(context.Light.Character.Specular, 1.0f),
					context.Light.Character.Position,

					vec4(context.Light.Stage.Ambient, 1.0f),
					vec4(context.Light.Stage.Diffuse, 1.0f),
					vec4(context.Light.Stage.Specular, 1.0f),
					context.Light.Stage.Position,

					context.IBL.Character.LightColor,
					context.IBL.Character.IrradianceRGB,
					context.IBL.Stage.LightColor,
					context.IBL.Stage.IrradianceRGB,
				});
		}
	}

	void SceneEditor::DrawDebugTestGui()
	{
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
				auto* nextEntity = ((i + 1) < sceneGraph.Entities.size()) ? sceneGraph.Entities[i + 1].get() : nullptr;

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
			auto applyA3D = [](auto& a3d, frame_t frame, auto& sceneGraph, auto& context)
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
								entityPattern.IDOverride = (index >= 0 && index < entityPattern.CachedIDs->size()) ? entityPattern.CachedIDs->at(index) : TxpID::Invalid;
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
					context.Camera.ViewPoint = A3DMgr::GetValueAt(a3dCamera.ViewPoint.Transform.Translation, frame);
					context.Camera.Interest = A3DMgr::GetValueAt(a3dCamera.Interest.Translation, frame);
					context.Camera.FieldOfView = A3DMgr::GetFieldOfViewAt(a3dCamera.ViewPoint, frame);
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
				iterateA3Ds(debug.StageEffA3Ds, debug.StageEffIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, context); });

			if (cameraController.Mode == CameraController3D::ControlMode::None)
				iterateA3Ds(debug.CamPVA3Ds, debug.CamPVIndex, [&](auto& a3d) { applyA3D(a3d, debug.Frame, sceneGraph, context); });

			if (Gui::Button("Camera Mode Orbit")) { context.Camera.FieldOfView = 90.0f; cameraController.Mode = CameraController3D::ControlMode::Orbit; }
			Gui::SameLine();
			if (Gui::Button("Camera Mode None")) { cameraController.Mode = CameraController3D::ControlMode::None; }

			if (Gui::Button("Set Screen Render IDs"))
			{
				for (auto& entity : sceneGraph.Entities)
				{
					for (const auto& material : entity->Obj->Materials)
					{
						if (auto txp = renderer3D->GetTxpFromTextureID(&material.Textures.Diffuse.TextureID); txp != nullptr && txp->Name.has_value())
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

								entity->Animation->ScreenRenderTextureID = material.Textures.Diffuse.TextureID;
							}
						}
					}
				}
			}
		}
	}

	void SceneEditor::TakeSceneRenderTargetScreenshot(D3D_RenderTarget& renderTarget)
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

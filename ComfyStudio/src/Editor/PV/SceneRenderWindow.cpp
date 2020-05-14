#include "SceneRenderWindow.h"
#include "CameraAxisIndication.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "Graphics/Auth3D/RayIntersection.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Editor
{
	using namespace Graphics;

	namespace
	{
		const Obj& GetDebugBoundingSphereObj(int subMeshIndex)
		{
			static std::array<std::unique_ptr<Obj>, 8> sphereObjs;
			constexpr std::array sphereColors =
			{
				vec4(0.00f, 0.00f, 0.75f, 0.50f),
				vec4(0.00f, 0.75f, 0.00f, 0.50f),
				vec4(0.00f, 0.75f, 0.75f, 0.50f),
				vec4(0.75f, 0.00f, 0.00f, 0.50f),
				vec4(0.75f, 0.00f, 0.75f, 0.50f),
				vec4(0.75f, 0.75f, 0.00f, 0.50f),
				vec4(0.75f, 0.75f, 0.75f, 0.50f),
				vec4(0.00f, 0.00f, 0.00f, 0.50f),
			};

			subMeshIndex %= sphereObjs.size();
			if (sphereObjs[subMeshIndex] != nullptr)
				return *sphereObjs[subMeshIndex];

			sphereObjs[subMeshIndex] = GenerateUploadDebugSphereObj(Sphere { vec3(0.0f), 1.0f }, sphereColors[subMeshIndex]);
			return *sphereObjs[subMeshIndex];
		}

		RenderCommand GetDebugBoungingSphereRenderCommand(const Transform& transform, const Sphere& sphere, int subMeshIndex)
		{
			RenderCommand command = {};
			command.SourceObj = &GetDebugBoundingSphereObj(subMeshIndex);

			command.Transform.Translation = transform.CalculateMatrix() * vec4(sphere.Center, 1.0f);
			command.Transform.Scale = vec3(sphere.Radius) * transform.Scale;
			command.Transform.Rotation = vec3(0.0f);

			return command;
		}

		void RenderDebugBoundingSpheres(GPU_Renderer3D* renderer3D, const ObjectEntity& entity)
		{
			if (entity.Obj->Debug.RenderBoundingSphere)
				renderer3D->Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, entity.Obj->BoundingSphere, 2));

			for (auto& mesh : entity.Obj->Meshes)
			{
				for (int i = 0; i < mesh.SubMeshes.size(); i++)
				{
					if (mesh.Debug.RenderBoundingSphere || mesh.SubMeshes[i].Debug.RenderBoundingSphere)
						renderer3D->Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, mesh.SubMeshes[i].BoundingSphere, i));
				}
			}
		}
	}

	SceneRenderWindow::SceneRenderWindow(SceneGraph& sceneGraph, SceneViewport& viewport, SceneParameters& scene, CameraController3D& cameraController, GPU_Renderer3D& renderer3D)
		: sceneGraph(&sceneGraph), viewport(&viewport), scene(&scene), cameraController(&cameraController), renderer3D(&renderer3D)
	{
	}

	void SceneRenderWindow::DrawGui()
	{
		RenderWindowBase::DrawGui();
	}

	void SceneRenderWindow::PostDrawGui()
	{
		if (drawCameraAxisIndicator)
		{
			constexpr float indicatorSize = 12.0f;
			constexpr float indicatorPadding = 20.0f;

			const vec2 textOffset = vec2(1.0f, indicatorSize * 0.85f);
			const vec2 indicatorCenter = GetRenderRegion().GetTR() + vec2(-(indicatorSize + indicatorPadding), +(indicatorSize + indicatorPadding));

			DrawCameraAxisIndicationGui(Gui::GetWindowDrawList(), viewport->Camera, indicatorCenter, indicatorSize, indicatorPadding, textOffset);
		}
	}

	void SceneRenderWindow::RenderScene()
	{
		cameraController->Update(viewport->Camera);

		// context->RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

		viewport->Camera.UpdateMatrices();
		renderer3D->Begin(*viewport, *scene);
		{
			auto isAnyReflection = std::any_of(sceneGraph->Entities.begin(), sceneGraph->Entities.end(), [](auto& e) { return e->IsReflection; });

			for (auto& entity : sceneGraph->Entities)
			{
				if (entity->IsVisible)
				{
					RenderCommand renderCommand;
					renderCommand.SourceObj = entity->Obj;
					renderCommand.SourceMorphObj = entity->MorphObj;
					renderCommand.Transform = entity->Transform;
					renderCommand.Flags.IsReflection = entity->IsReflection;

					renderCommand.Animation = entity->Dynamic.get();

					if (entity->SilhouetteOutline)
						renderCommand.Flags.SilhouetteOutline = true;

#if 1 // DEBUG:
					if (entity->Tag == 'chr' || entity->Tag == 'obj')
						renderCommand.Flags.CastsShadow = true;
#endif

					renderer3D->Draw(renderCommand);

#if 1 // DEBUG:
					// DEBUG: Quick and dirt hack for testing sake, not accurate of course
					if (isAnyReflection && entity->Tag == 'chr')
					{
						renderCommand.Flags.IsReflection = true;
						renderCommand.Transform.Translation.y = 0.0f - renderCommand.Transform.Translation.y;
						renderCommand.Transform.Rotation.x = 0.0f - renderCommand.Transform.Rotation.x;
						renderCommand.Transform.Rotation.z = 180.0f - renderCommand.Transform.Rotation.z;
						renderer3D->Draw(renderCommand);
					}
#endif

#if 1 // DEBUG:
					RenderDebugBoundingSpheres(renderer3D, *entity);
#endif
				}
			}

			if (cameraController->Visualization.VisualizeInterest && cameraController->Visualization.InterestSphereObj != nullptr)
				renderer3D->Draw(RenderCommand(*cameraController->Visualization.InterestSphereObj, viewport->Camera.Interest));
		}
		renderer3D->End();
	}

	void SceneRenderWindow::OnUpdateInput()
	{
#if 0 // DEBUG:
		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			for (auto& entity : sceneGraph->Entities)
				entity->SilhouetteOutline = false;

			if (Gui::IsMouseDown(1))
			{
				const vec3 ray = viewport->Camera.CalculateRayDirection(GetRelativeMouse() / GetRenderRegion().GetSize());
				const vec3 viewPoint = viewport->Camera.ViewPoint;

				float closestDistance = 0.0f;
				ObjectEntity* closestEntity = nullptr;

				for (auto& entity : sceneGraph->Entities)
				{
					if (!entity->IsVisible || entity->IsReflection)
						continue;

					// NOTE: Optionally ignore *all* camera intersecting entities
					// if ((entity->Obj->BoundingSphere * entity->Transform).Contains(viewPoint))
					// 	continue;

					float intersectionDistance = 0.0f;
					if (!Intersects(viewPoint, ray, *entity->Obj, entity->Transform, intersectionDistance))
						continue;

					// NOTE: Add a bias to camera intersecting entities so the stage ground / sky only get picked if nothing else is in between
					const Sphere transformedSphere = (entity->Obj->BoundingSphere * entity->Transform);
					if (transformedSphere.Contains(viewPoint))
						intersectionDistance += transformedSphere.Radius;

					if (intersectionDistance < closestDistance || closestEntity == nullptr)
					{
						closestDistance = intersectionDistance;
						closestEntity = entity.get();
					}
				}

				if (closestEntity != nullptr)
					closestEntity->SilhouetteOutline = true;
			}
		}
#endif
	}

	void SceneRenderWindow::OnUpdate()
	{
	}

	void SceneRenderWindow::OnRender()
	{
		RenderScene();
	}

	void SceneRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		vec2 renderRegionSize = GetRenderRegion().GetSize();
		viewport->Camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		viewport->Parameters.RenderResolution = renderRegionSize;
	}

	GPU_RenderTarget* SceneRenderWindow::GetExternalRenderTarget()
	{
		return (viewport == nullptr) ? nullptr : &viewport->Data.Output.RenderTarget;
	}
}

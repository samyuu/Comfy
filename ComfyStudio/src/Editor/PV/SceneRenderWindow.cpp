#include "SceneRenderWindow.h"
#include "CameraAxisIndication.h"
#include "Graphics/Auth3D/Misc/DebugObj.h"
#include "Render/Misc/RayIntersection.h"
#include "Editor/Core/Theme.h"

namespace Comfy::Studio::Editor
{
	using namespace Graphics;
	using namespace Render;

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

			sphereObjs[subMeshIndex] = GenerateDebugSphereObj(Sphere { vec3(0.0f), 1.0f }, sphereColors[subMeshIndex]);
			return *sphereObjs[subMeshIndex];
		}

		RenderCommand3D GetDebugBoungingSphereRenderCommand(const Transform& transform, const Sphere& sphere, int subMeshIndex)
		{
			RenderCommand3D command = {};
			command.SourceObj = &GetDebugBoundingSphereObj(subMeshIndex);

			command.Transform.Translation = transform.CalculateMatrix() * vec4(sphere.Center, 1.0f);
			command.Transform.Scale = vec3(sphere.Radius) * transform.Scale;
			command.Transform.Rotation = vec3(0.0f);

			return command;
		}

		void RenderDebugBoundingSpheres(Renderer3D& renderer3D, const ObjectEntity& entity)
		{
			if (entity.Obj->Debug.RenderBoundingSphere)
				renderer3D.Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, entity.Obj->BoundingSphere, 2));

			for (auto& mesh : entity.Obj->Meshes)
			{
				for (int i = 0; i < mesh.SubMeshes.size(); i++)
				{
					if (mesh.Debug.RenderBoundingSphere || mesh.SubMeshes[i].Debug.RenderBoundingSphere)
						renderer3D.Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, mesh.SubMeshes[i].BoundingSphere, i));
				}
			}
		}
	}

	SceneRenderWindow::SceneRenderWindow(SceneGraph& sceneGraph, Render::PerspectiveCamera& camera, Render::Renderer3D& renderer, Render::SceneParam3D& sceneParam, CameraController3D& cameraController)
		: sceneGraph(sceneGraph), camera(camera), renderer(renderer), sceneParam(sceneParam), cameraController(cameraController)
	{
		renderTarget = Renderer3D::CreateRenderTarget();
	}

	ImTextureID SceneRenderWindow::GetTextureID() const
	{
		return (renderTarget != nullptr) ? renderTarget->GetTextureID() : nullptr;
	}

	void SceneRenderWindow::RenderScene()
	{
		cameraController.Update(camera);

		renderer.Begin(camera, *renderTarget, sceneParam);
		{
			auto isAnyReflection = std::any_of(sceneGraph.Entities.begin(), sceneGraph.Entities.end(), [](auto& e) { return e->IsReflection; });

			for (auto& entity : sceneGraph.Entities)
			{
				if (!entity->IsVisible)
					continue;

				RenderCommand3D renderCommand;
				renderCommand.SourceObj = entity->Obj;
				renderCommand.Transform = entity->Transform;
				renderCommand.Flags.IsReflection = entity->IsReflection;

				renderCommand.Dynamic = entity->Dynamic.get();

				if (entity->SilhouetteOutline)
					renderCommand.Flags.SilhouetteOutline = true;

#if 1 // DEBUG:
				if (entity->Tag == 'chr' || entity->Tag == 'obj')
					renderCommand.Flags.CastsShadow = true;
#endif

				renderer.Draw(renderCommand);

#if 1 // DEBUG:
				// DEBUG: Quick and dirt hack for testing sake, not accurate of course
				if (isAnyReflection && entity->Tag == 'chr')
				{
					renderCommand.Flags.IsReflection = true;
					renderCommand.Transform.Translation.y = 0.0f - renderCommand.Transform.Translation.y;
					renderCommand.Transform.Rotation.x = 0.0f - renderCommand.Transform.Rotation.x;
					renderCommand.Transform.Rotation.z = 180.0f - renderCommand.Transform.Rotation.z;
					renderer.Draw(renderCommand);
				}
#endif

#if 1 // DEBUG:
				RenderDebugBoundingSpheres(renderer, *entity);
#endif
			}

			if (cameraController.Visualization.VisualizeInterest && cameraController.Visualization.InterestSphereObj != nullptr)
				renderer.Draw(RenderCommand3D(*cameraController.Visualization.InterestSphereObj, camera.Interest));
		}
		renderer.End();
	}

	Render::RenderTarget3D* SceneRenderWindow::GetRenderTarget()
	{
		return renderTarget.get();
	}

	i64 SceneRenderWindow::GetLastFocusedFrameCount() const
	{
		return lastFocusedFrameCount;
	}

	bool SceneRenderWindow::GetRequestsDuplication() const
	{
		return requestsDuplication;
	}

	ImGuiWindowFlags SceneRenderWindow::GetRenderTextureChildWindowFlags() const
	{
		return ImGuiWindowFlags_None;
	}

	void SceneRenderWindow::PreBeginWindow()
	{
		hasInputFocus = false;
		requestsDuplication = false;
	}

	void SceneRenderWindow::OnFirstFrame()
	{
	}

	void SceneRenderWindow::PreRenderTextureGui()
	{
		return;
	}

	void SceneRenderWindow::PostRenderTextureGui()
	{
		Gui::WindowContextMenu("ContextMenu##SceneRenderWindow", [&]
		{
			if (Gui::MenuItem("Duplicate Viewport"))
				requestsDuplication = true;
		});

		if (drawCameraAxisIndicator)
		{
			constexpr float indicatorSize = 12.0f;
			constexpr float indicatorPadding = 20.0f;

			const auto textOffset = vec2(1.0f, indicatorSize * 0.85f);
			const auto indicatorCenter = GetRenderRegion().GetTR() + vec2(-(indicatorSize + indicatorPadding), +(indicatorSize + indicatorPadding));

			DrawCameraAxisIndicationGui(Gui::GetWindowDrawList(), camera, indicatorCenter, indicatorSize, indicatorPadding, textOffset);
		}
	}

	void SceneRenderWindow::OnResize(ivec2 newSize)
	{
		const auto renderRegionSize = vec2(GetRenderRegion().GetSize());
		camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		renderTarget->Param.RenderResolution = renderRegionSize;
	}

	void SceneRenderWindow::OnRender()
	{
		if (hasInputFocus = Gui::IsWindowFocused())
			lastFocusedFrameCount = Gui::GetFrameCount();

		UpdateInputRayTest();
		RenderScene();
	}

	void SceneRenderWindow::UpdateInputRayTest()
	{
#if 0
		if (!Gui::IsWindowFocused() || !Gui::IsWindowHovered())
			return;

		for (auto& entity : sceneGraph.Entities)
			entity->SilhouetteOutline = false;

		if (Gui::IsMouseDown(1))
		{
			const vec3 ray = camera.CalculateRayDirection(GetRelativeMouse() / GetRenderRegion().GetSize());
			const vec3 viewPoint = camera.ViewPoint;

			float closestDistance = 0.0f;
			ObjectEntity* closestEntity = nullptr;

			for (auto& entity : sceneGraph.Entities)
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
#endif
	}
}

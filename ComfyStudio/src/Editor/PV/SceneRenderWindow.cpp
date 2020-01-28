#include "SceneRenderWindow.h"
#include "CameraAxisIndication.h"
#include "Graphics/Auth3D/DebugObj.h"
#include "Graphics/Auth3D/RayIntersection.h"
#include "Editor/Core/Theme.h"

namespace Editor
{
	using namespace Graphics;

	namespace
	{
		Obj* GetDebugBoundingSphereObj(int subMeshIndex)
		{
			static std::array<UniquePtr<Obj>, 8> sphereObjs;
			constexpr std::array sphereColors =
			{
				vec4(0.00f, 0.00f, 0.75f, 0.25f),
				vec4(0.00f, 0.75f, 0.00f, 0.25f),
				vec4(0.00f, 0.75f, 0.75f, 0.25f),
				vec4(0.75f, 0.00f, 0.00f, 0.25f),
				vec4(0.75f, 0.00f, 0.75f, 0.25f),
				vec4(0.75f, 0.75f, 0.00f, 0.25f),
				vec4(0.75f, 0.75f, 0.75f, 0.25f),
				vec4(0.00f, 0.00f, 0.00f, 0.25f),
			};

			subMeshIndex %= sphereObjs.size();
			if (sphereObjs[subMeshIndex] != nullptr)
				return sphereObjs[subMeshIndex].get();

			sphereObjs[subMeshIndex] = GenerateUploadDebugSphereObj(Sphere { vec3(0.0f), 1.0f }, sphereColors[subMeshIndex]);
			return sphereObjs[subMeshIndex].get();
		}

		RenderCommand GetDebugBoungingSphereRenderCommand(const Transform& transform, const Sphere& sphere, int subMeshIndex)
		{
			RenderCommand command = {};
			command.SourceObj = GetDebugBoundingSphereObj(subMeshIndex);

			command.Transform.Translation = transform.CalculateMatrix() * vec4(sphere.Center, 1.0f);
			command.Transform.Scale = vec3(sphere.Radius) * transform.Scale;
			command.Transform.Rotation = vec3(0.0f);

			return command;
		}

		void RenderDebugBoundingSpheres(D3D_Renderer3D* renderer3D, const ObjectEntity& entity)
		{
			if (entity.Obj->Debug.RenderBoundingSphere)
				renderer3D->Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, entity.Obj->BoundingSphere, 2));

			for (auto& mesh : entity.Obj->Meshes)
			{
				if (mesh.Debug.RenderBoundingSphere)
				{
					for (int i = 0; i < mesh.SubMeshes.size(); i++)
						renderer3D->Draw(GetDebugBoungingSphereRenderCommand(entity.Transform, mesh.SubMeshes[i].BoundingSphere, i));
				}
			}
		}
	}

	SceneRenderWindow::SceneRenderWindow(SceneGraph& sceneGraph, SceneContext& context, CameraController3D& cameraController, D3D_Renderer3D& renderer3D)
		: sceneGraph(&sceneGraph), context(&context), cameraController(&cameraController), renderer3D(&renderer3D)
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

			DrawCameraAxisIndicationGui(Gui::GetWindowDrawList(), context->Camera, indicatorCenter, indicatorSize, indicatorPadding, textOffset);
		}
	}

	void SceneRenderWindow::OnUpdateInput()
	{
#if 0 // DEBUG:
		if (Gui::IsWindowFocused() && Gui::IsWindowHovered())
		{
			for (auto& entity : sceneGraph->Entities)
				entity->Obj->Debug.WireframeOverlay = false;

			if (Gui::IsMouseDown(1))
			{
				const vec3 ray = context->Camera.CalculateRayDirection(GetRelativeMouse() / GetRenderRegion().GetSize());
				const vec3 viewPoint = context->Camera.ViewPoint;

				float closestDistance = 0.0f;
				ObjectEntity* closestEntity = nullptr;

				for (auto& entity : sceneGraph->Entities)
				{
					if (!entity->IsVisible || entity->IsReflection)
						continue;

					float intersectionDistance = 0.0f;
					if (!Intersects(viewPoint, ray, *entity->Obj, entity->Transform, intersectionDistance))
						continue;

					if (intersectionDistance < closestDistance || closestEntity == nullptr)
					{
						closestDistance = intersectionDistance;
						closestEntity = entity.get();
					}
				}

				if (closestEntity != nullptr)
					closestEntity->Obj->Debug.WireframeOverlay = true;
			}
		}
#endif
	}

	void SceneRenderWindow::OnUpdate()
	{
	}

	void SceneRenderWindow::OnRender()
	{
		cameraController->Update(context->Camera);

		context->RenderData.Output.RenderTarget = owningRenderTarget.get();
		// context->RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

		context->Camera.UpdateMatrices();
		renderer3D->Begin(*context);
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

					renderCommand.Animation = entity->Animation.get();

#if 0 // DEBUG:
					if (entity->Obj->Debug.WireframeOverlay)
						renderCommand.Flags.SilhouetteOutline = true;
#endif

					renderer3D->Draw(renderCommand);

					// DEBUG: Quick and dirt hack for testing sake, not accurate of course
					if (isAnyReflection && entity->Tag == 'chr')
					{
						renderCommand.Flags.IsReflection = true;
						renderCommand.Transform.Translation.y = 0.0f - renderCommand.Transform.Translation.y;
						renderCommand.Transform.Rotation.x = 0.0f - renderCommand.Transform.Rotation.x;
						renderCommand.Transform.Rotation.z = 180.0f - renderCommand.Transform.Rotation.z;
						renderer3D->Draw(renderCommand);
					}

#if 1 // DEBUG:
					RenderDebugBoundingSpheres(renderer3D, *entity);
#endif
				}
			}

			if (cameraController->Visualization.VisualizeInterest && cameraController->Visualization.InterestSphereObj != nullptr)
				renderer3D->Draw(RenderCommand(*cameraController->Visualization.InterestSphereObj, context->Camera.Interest));
		}
		renderer3D->End();
	}

	void SceneRenderWindow::OnResize(ivec2 size)
	{
		RenderWindowBase::OnResize(size);

		vec2 renderRegionSize = GetRenderRegion().GetSize();
		context->Camera.AspectRatio = renderRegionSize.x / renderRegionSize.y;

		context->RenderParameters.RenderResolution = renderRegionSize;
	}
}

#include "SceneRenderWindow.h"
#include "CameraAxisIndication.h"
#include "Graphics/Auth3D/DebugObj.h"
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
		return;
	}

	void SceneRenderWindow::OnUpdate()
	{
	}

	void SceneRenderWindow::OnRender()
	{
		cameraController->Update(context->Camera);

		context->RenderData.OutputRenderTarget = renderTarget.get();
		// context->RenderParameters.ClearColor = GetColorVec4(EditorColor_BaseClear);

		context->Camera.UpdateMatrices();
		renderer3D->Begin(*context);
		{
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
					renderer3D->Draw(renderCommand);

					if (true)
						RenderDebugBoundingSpheres(renderer3D, *entity);
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

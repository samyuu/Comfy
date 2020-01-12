#include "SceneRenderWindow.h"
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

		RenderCommand GetDebugBoungingSphereRenderCommand(const ObjectEntity& entity, const SubMesh& subMesh, int subMeshIndex)
		{
			RenderCommand command = {};
			command.SourceObj = GetDebugBoundingSphereObj(subMeshIndex);

			command.Transform = entity.Transform;
			command.Transform.Translation += subMesh.BoundingSphere.Center;
			command.Transform.Scale *= subMesh.BoundingSphere.Radius;

			return command;
		}

		void RenderDebugBoundingSpheres(D3D_Renderer3D* renderer3D, const ObjectEntity& entity)
		{
			for (auto& mesh : entity.Obj->Meshes)
			{
				if (mesh.Debug.RenderBoundingSphere)
				{
					for (int i = 0; i < mesh.SubMeshes.size(); i++)
						renderer3D->Draw(GetDebugBoungingSphereRenderCommand(entity, mesh.SubMeshes[i], i));
				}
			}
		}
	}

	SceneRenderWindow::SceneRenderWindow(SceneGraph& sceneGraph, SceneContext& context, CameraController3D& cameraController, D3D_Renderer3D& renderer3D)
		: sceneGraph(&sceneGraph), context(&context), cameraController(&cameraController), renderer3D(&renderer3D)
	{
	}

	void SceneRenderWindow::Initialize()
	{
		RenderWindowBase::Initialize();
	}

	void SceneRenderWindow::DrawGui()
	{
		RenderWindowBase::DrawGui();
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
					renderCommand.Transform = entity->Transform;
					renderCommand.Flags.IsReflection = entity->IsReflection;

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

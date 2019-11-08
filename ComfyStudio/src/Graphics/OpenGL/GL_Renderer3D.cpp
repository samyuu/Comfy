#include "GL_Renderer3D.h"

namespace Graphics
{
	void GL_Renderer3D::Initialize()
	{
		simpleShader = MakeUnique<GL_SimpleShader>();
		simpleShader->Initialize();
	}

	void GL_Renderer3D::Begin(const PerspectiveCamera& camera)
	{
		perspectiveCamera = &camera;

		simpleShader->Bind();
		simpleShader->SetUniform(simpleShader->View, perspectiveCamera->GetViewMatrix());
		simpleShader->SetUniform(simpleShader->Projection, perspectiveCamera->GetProjectionMatrix());
	}

	void GL_Renderer3D::Draw(const Obj* object, const vec3& position)
	{
		assert(object != nullptr);

		mat4 modelMatrix = glm::translate(mat4(1.0f), position);
		simpleShader->SetUniform(simpleShader->Model, modelMatrix);

		for (auto& mesh : object->Meshes)
		{
			mesh->GraphicsBuffers.VertexArray->Bind();

			for (auto& subMesh : mesh->SubMeshes)
			{
				subMesh->GraphicsIndexBuffer->Bind();

				RenderCommand::DrawElements(
					subMesh->Primitive,
					static_cast<int32_t>(subMesh->Indices.size()),
					subMesh->GraphicsIndexBuffer->GetGLIndexType(),
					nullptr);
			}
		}
	}

	void GL_Renderer3D::End()
	{
	}
}
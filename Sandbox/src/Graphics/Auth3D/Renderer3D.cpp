#include "Renderer3D.h"

namespace Graphics::Auth3D
{
	void Renderer3D::Initialize()
	{
		simpleShader = MakeUnique<SimpleShader>();
		simpleShader->Initialize();
	}

	void Renderer3D::Begin(const PerspectiveCamera& camera)
	{
		this->camera = &camera;


		simpleShader->Bind();
		simpleShader->SetUniform(simpleShader->View, this->camera->GetViewMatrix());
		simpleShader->SetUniform(simpleShader->Projection, this->camera->GetProjectionMatrix());

	}

	void Renderer3D::Draw(const Obj* object, const vec3& position)
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
					subMesh->Indices.size(),
					subMesh->GraphicsIndexBuffer->GetGLIndexType(),
					nullptr);
			}
		}
	}

	void Renderer3D::End()
	{
	}
}
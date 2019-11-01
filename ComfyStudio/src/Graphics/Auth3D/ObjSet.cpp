#include "ObjSet.h"

namespace Graphics
{
	template <class T>
	static inline void UploadInitializeBufferIfAttribute(bool hasAttribute, RefPtr<GL_VertexBuffer>& vertexBuffer, std::vector<T>& vertexData)
	{
		if (!hasAttribute)
			return;

		vertexBuffer = MakeRef<GL_VertexBuffer>(BufferUsage::StaticDraw);
		vertexBuffer->InitializeID();
		vertexBuffer->Bind();
		vertexBuffer->Upload(vertexData.size() * sizeof(T), vertexData.data());
	}

	void ObjSet::UploadAll()
	{
		for (RefPtr<Obj>& obj : objects)
		{
			for (RefPtr<Mesh>& mesh : obj->Meshes)
			{
				UploadInitializeBufferIfAttribute(mesh->VertexAttributes.Position, mesh->GraphicsBuffers.PositionBuffer, mesh->VertexData.Positions);
				UploadInitializeBufferIfAttribute(mesh->VertexAttributes.Normal, mesh->GraphicsBuffers.NormalBuffer, mesh->VertexData.Normals);

				mesh->GraphicsBuffers.VertexArray = MakeRef<GL_VertexArray>();
				mesh->GraphicsBuffers.VertexArray->InitializeID();
				mesh->GraphicsBuffers.VertexArray->Bind();

				BufferLayout layout = 
				{ 
					{ ShaderDataType::Vec3, "in_Position", false, mesh->GraphicsBuffers.PositionBuffer.get() },
					{ ShaderDataType::Vec3, "in_Normal", false, mesh->GraphicsBuffers.NormalBuffer.get() },
				};
				mesh->GraphicsBuffers.VertexArray->SetLayout(layout, false);

				for (RefPtr<SubMesh>& subMesh : mesh->SubMeshes)
				{
					subMesh->GraphicsIndexBuffer = MakeRef<GL_IndexBuffer>(BufferUsage::StaticDraw, IndexType::UnsignedShort);
					subMesh->GraphicsIndexBuffer->InitializeID();
					subMesh->GraphicsIndexBuffer->Bind();
					subMesh->GraphicsIndexBuffer->Upload(subMesh->Indices.size() * sizeof(uint16_t), subMesh->Indices.data());
				}
			}
		}
	}
}
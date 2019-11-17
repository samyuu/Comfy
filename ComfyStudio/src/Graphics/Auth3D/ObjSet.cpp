#include "ObjSet.h"

namespace Graphics
{
	template <class T>
	void ObjSet::InitializeBufferIfAttribute(bool hasAttribute, UniquePtr<D3D_StaticVertexBuffer>& vertexBuffer, std::vector<T>& vertexData, Mesh* mesh, const char* name)
	{
		if (!hasAttribute)
			return;

		vertexBuffer = MakeUnique<D3D_StaticVertexBuffer>(vertexData.size() * sizeof(T), vertexData.data(), sizeof(T));
		D3D_SetObjectDebugName(vertexBuffer->GetBuffer(), "<%s> %s::%sBuffer", Name.c_str(), mesh->Name.c_str(), name);
	}

	void ObjSet::UploadAll()
	{
		for (auto& obj : objects)
		{
			for (auto& mesh : obj->Meshes)
			{
				InitializeBufferIfAttribute(mesh->VertexAttributes.Position, mesh->GraphicsBuffers.PositionBuffer, mesh->VertexData.Positions, mesh.get(), "Posititon");
				InitializeBufferIfAttribute(mesh->VertexAttributes.Normal, mesh->GraphicsBuffers.NormalBuffer, mesh->VertexData.Normals, mesh.get(), "Normal");

				// TODO: Need shader signature, should all 3d shaders have the same input together with a constantbuffer layout flag
				//		 then create this layout using an unused dummy vertex shader with the same layout (?) (other shaders could even #include that dummy shader) MonkaHmm
				// mesh->GraphicsBuffers.InputLayout = MakeUnique<D3D_InputLayout>(...);

				uint32_t subMeshIndex = 0;
				for (auto& subMesh : mesh->SubMeshes)
				{
					subMesh->GraphicsIndexBuffer = MakeUnique<D3D_StaticIndexBuffer>(subMesh->Indices.size() * sizeof(uint16_t), subMesh->Indices.data(), IndexType::UInt16);

					D3D_SetObjectDebugName(subMesh->GraphicsIndexBuffer->GetBuffer(), "<%s> %s[%d]::IndexBuffer", Name.c_str(), mesh->Name.c_str(), subMeshIndex);
					subMeshIndex++;
				}
			}
		}
	}
}

#include "ObjSet.h"

namespace Graphics
{
	namespace
	{
		template <class T>
		void InitializeBufferIfAttribute(Mesh& mesh, VertexAttribute attribute, std::vector<T>& vertexData, const char* objSetName, const char* bufferName)
		{
			auto& vertexBuffer = mesh.GraphicsAttributeBuffers[attribute];

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
			{
				vertexBuffer = nullptr;
				return;
			}

			vertexBuffer = MakeUnique<D3D_StaticVertexBuffer>(vertexData.size() * sizeof(T), vertexData.data(), sizeof(T));
			D3D_SetObjectDebugName(vertexBuffer->GetBuffer(), "<%s> %s::%sBuffer", objSetName, mesh.Name, bufferName);
		}
	}

	void ObjSet::UploadAll()
	{
		for (auto& obj : objects)
		{
			for (auto& mesh : obj.Meshes)
			{
				InitializeBufferIfAttribute(mesh, VertexAttribute_Position, mesh.VertexData.Positions, Name.c_str(), "Posititon");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Normal, mesh.VertexData.Normals, Name.c_str(), "Normal");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, Name.c_str(), "Tangent");
				InitializeBufferIfAttribute(mesh, VertexAttribute_0x3, mesh.VertexData.Attribute_0x3, Name.c_str(), "Attribute_0x3");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], Name.c_str(), "TextureCoordinate[0]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], Name.c_str(), "TextureCoordinate[1]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], Name.c_str(), "TextureCoordinate[2]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], Name.c_str(), "TextureCoordinate[3]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], Name.c_str(), "Color[0]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], Name.c_str(), "Color[1]");
				InitializeBufferIfAttribute(mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, Name.c_str(), "BoneWeight");
				InitializeBufferIfAttribute(mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, Name.c_str(), "BoneIndex");

				uint32_t subMeshIndex = 0;
				for (auto& subMesh : mesh.SubMeshes)
				{
					subMesh.GraphicsIndexBuffer = MakeUnique<D3D_StaticIndexBuffer>(subMesh.Indices.size() * sizeof(uint16_t), subMesh.Indices.data(), IndexType::UInt16);

					D3D_SetObjectDebugName(subMesh.GraphicsIndexBuffer->GetBuffer(), "<%s> %s[%d]::IndexBuffer", Name.c_str(), mesh.Name, subMeshIndex);
					subMeshIndex++;
				}
			}
		}
	}
}

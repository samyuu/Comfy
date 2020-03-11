#include "ObjSet.h"

namespace Comfy::Graphics
{
	namespace
	{
		template <typename T>
		void InitializeBufferIfAttribute(Mesh& mesh, VertexAttribute attribute, std::vector<T>& vertexData, const char* objSetName, const char* bufferName)
		{
			auto& vertexBuffer = mesh.D3D_VertexBuffers[attribute];

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

	void Obj::Upload()
	{
		for (auto& mesh : Meshes)
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
				subMesh.D3D_IndexBuffer = MakeUnique<D3D_StaticIndexBuffer>(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat());

				D3D_SetObjectDebugName(subMesh.D3D_IndexBuffer->GetBuffer(), "<%s> %s[%d]::IndexBuffer", Name.c_str(), mesh.Name, subMeshIndex);
				subMeshIndex++;
			}
		}
	}

	void ObjSet::UploadAll()
	{
		for (auto& obj : objects)
			obj.Upload();
	}

	UniquePtr<ObjSet> ObjSet::MakeUniqueReadParseUpload(std::string_view filePath)
	{
		auto objSet = MakeUnique<ObjSet>();
		objSet->Load(std::string(filePath));
		objSet->UploadAll();
		return objSet;
	}

	IndexFormat SubMesh::GetIndexFormat() const
	{
		return static_cast<IndexFormat>(Indices.index());
	}

	std::vector<uint8_t>* SubMesh::GetIndicesU8()
	{
		return std::get_if<std::vector<uint8_t>>(&Indices);
	}

	const std::vector<uint8_t>* SubMesh::GetIndicesU8() const
	{
		return std::get_if<std::vector<uint8_t>>(&Indices);
	}

	std::vector<uint16_t>* SubMesh::GetIndicesU16()
	{
		return std::get_if<std::vector<uint16_t>>(&Indices);
	}

	const std::vector<uint16_t>* SubMesh::GetIndicesU16() const
	{
		return std::get_if<std::vector<uint16_t>>(&Indices);
	}

	std::vector<uint32_t>* SubMesh::GetIndicesU32()
	{
		return std::get_if<std::vector<uint32_t>>(&Indices);
	}

	const std::vector<uint32_t>* SubMesh::GetIndicesU32() const
	{
		return std::get_if<std::vector<uint32_t>>(&Indices);
	}

	const size_t SubMesh::GetIndexCount() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return indices->size();

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return indices->size();

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return indices->size();

		return 0;
	}

	const void* SubMesh::GetRawIndices() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return static_cast<const void*>(indices->data());

		return nullptr;
	}

	size_t SubMesh::GetRawIndicesByteSize() const
	{
		if (auto indices = GetIndicesU8(); indices != nullptr)
			return indices->size() * sizeof(uint8_t);

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return indices->size() * sizeof(uint16_t);

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return indices->size() * sizeof(uint32_t);

		return 0;
	}

	Material& SubMesh::GetMaterial(Obj& obj)
	{
		assert(MaterialIndex < obj.Materials.size());
		return obj.Materials[MaterialIndex];
	}

	const Material& SubMesh::GetMaterial(const Obj& obj) const
	{
		assert(MaterialIndex < obj.Materials.size());
		return obj.Materials[MaterialIndex];
	}
}

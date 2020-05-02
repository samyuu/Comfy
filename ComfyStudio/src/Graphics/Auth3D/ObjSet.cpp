#include "ObjSet.h"
#include "IO/File.h"

namespace Comfy::Graphics
{
	namespace
	{
		template <typename T>
		void InitializeBufferIfAttribute(Mesh& mesh, VertexAttribute attribute, std::vector<T>& vertexData, const char* objSetName, const char* bufferName)
		{
			auto& vertexBuffer = mesh.GPU_VertexBuffers[attribute];

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
			{
				vertexBuffer = nullptr;
				return;
			}

			const char* debugName = nullptr;

#if COMFY_D3D11_DEBUG_NAMES
			char debugNameBuffer[128];
			sprintf_s(debugNameBuffer,"<%s> %s::%sBuffer", objSetName, mesh.Name.data(), bufferName);
			debugName = debugNameBuffer;
#endif

			vertexBuffer = GPU::MakeVertexBuffer(vertexData.size() * sizeof(T), vertexData.data(), sizeof(T), debugName);
		}
	}

	void Obj::Upload()
	{
		for (auto& mesh : Meshes)
		{
			InitializeBufferIfAttribute(mesh, VertexAttribute_Position, mesh.VertexData.Positions, Name.c_str(), "Posititon");
			InitializeBufferIfAttribute(mesh, VertexAttribute_Normal, mesh.VertexData.Normals, Name.c_str(), "Normal");
			InitializeBufferIfAttribute(mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, Name.c_str(), "Tangent");
			InitializeBufferIfAttribute(mesh, VertexAttribute_0x3, mesh.VertexData.Reserved0x3, Name.c_str(), "Reserved0x3");
			InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], Name.c_str(), "TextureCoordinate[0]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], Name.c_str(), "TextureCoordinate[1]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], Name.c_str(), "TextureCoordinate[2]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], Name.c_str(), "TextureCoordinate[3]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], Name.c_str(), "Color[0]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], Name.c_str(), "Color[1]");
			InitializeBufferIfAttribute(mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, Name.c_str(), "BoneWeight");
			InitializeBufferIfAttribute(mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, Name.c_str(), "BoneIndex");

			u32 subMeshIndex = 0;
			for (auto& subMesh : mesh.SubMeshes)
			{
				const char* debugName = nullptr;

#if COMFY_D3D11_DEBUG_NAMES
				char debugNameBuffer[128];
				sprintf_s(debugNameBuffer, "<%s> %s[%d]::IndexBuffer", Name.c_str(), mesh.Name.data(), subMeshIndex++);
				debugName = debugNameBuffer;
#endif

				subMesh.GPU_IndexBuffer = GPU::MakeIndexBuffer(subMesh.GetRawIndicesByteSize(), subMesh.GetRawIndices(), subMesh.GetIndexFormat(), debugName);
			}
		}
	}

	void ObjSet::UploadAll()
	{
		for (auto& obj : objects)
			obj.Upload();
	}

	std::unique_ptr<ObjSet> ObjSet::MakeUniqueReadParseUpload(std::string_view filePath)
	{
		auto objSet = IO::File::Load<ObjSet>(filePath);
		if (objSet != nullptr)
			objSet->UploadAll();
		return objSet;
	}

	IndexFormat SubMesh::GetIndexFormat() const
	{
		return static_cast<IndexFormat>(Indices.index());
	}

	std::vector<u8>* SubMesh::GetIndicesU8()
	{
		return std::get_if<std::vector<u8>>(&Indices);
	}

	const std::vector<u8>* SubMesh::GetIndicesU8() const
	{
		return std::get_if<std::vector<u8>>(&Indices);
	}

	std::vector<u16>* SubMesh::GetIndicesU16()
	{
		return std::get_if<std::vector<u16>>(&Indices);
	}

	const std::vector<u16>* SubMesh::GetIndicesU16() const
	{
		return std::get_if<std::vector<u16>>(&Indices);
	}

	std::vector<u32>* SubMesh::GetIndicesU32()
	{
		return std::get_if<std::vector<u32>>(&Indices);
	}

	const std::vector<u32>* SubMesh::GetIndicesU32() const
	{
		return std::get_if<std::vector<u32>>(&Indices);
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
			return indices->size() * sizeof(u8);

		if (auto indices = GetIndicesU16(); indices != nullptr)
			return indices->size() * sizeof(u16);

		if (auto indices = GetIndicesU32(); indices != nullptr)
			return indices->size() * sizeof(u32);

		return 0;
	}
}

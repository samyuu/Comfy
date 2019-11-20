#include "ObjSet.h"
#include "FileSystem/BinaryReader.h"

using namespace FileSystem;

namespace Graphics
{
	namespace
	{
		vec3 ReadVec3(BinaryReader& reader)
		{
			return vec3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
		}

		Sphere ReadSphere(BinaryReader& reader)
		{
			return { ReadVec3(reader), reader.ReadFloat() };
		}

		Box ReadBox(BinaryReader& reader)
		{
			return { ReadVec3(reader), ReadVec3(reader) };
		}

		template <class T>
		void CheckReadVertexData(BinaryReader& reader, Mesh& mesh, VertexAttribute_Enum attribute, std::vector<T>& vector, void** attributePointers, void* baseAddress)
		{
			void* attributePointer = attributePointers[attribute];
			if (attributePointer == nullptr)
				return;

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
				return;

			reader.ReadAt(attributePointer, baseAddress, [&vector, &mesh](BinaryReader& reader)
			{
				vector.resize(mesh.VertexData.VertexCount);
				reader.Read(vector.data(), vector.size() * sizeof(T));
			});
		}
	}

	void Obj::Read(BinaryReader& reader)
	{
		void* objBaseAddress = reader.GetPositionPtr();

		uint32_t unknown0 = reader.ReadUInt32();
		uint32_t unknown1 = reader.ReadUInt32();
		BoundingSphere = ReadSphere(reader);

		uint32_t meshCount = reader.ReadUInt32();
		void* meshesPtr = reader.ReadPtr();
		if (meshCount > 0 && meshesPtr != nullptr)
		{
			Meshes.resize(meshCount);
			reader.ReadAt(meshesPtr, objBaseAddress, [this, objBaseAddress](BinaryReader& reader)
			{
				for (auto& mesh : Meshes)
				{
					uint32_t unknown0 = reader.ReadUInt32();
					mesh.BoundingSphere = ReadSphere(reader);

					uint32_t subMeshCount = reader.ReadUInt32();
					void* subMeshesPtr = reader.ReadPtr();
					if (subMeshCount > 0 && subMeshesPtr != nullptr)
					{
						mesh.SubMeshes.resize(subMeshCount);
						reader.ReadAt(subMeshesPtr, objBaseAddress, [&mesh, objBaseAddress](BinaryReader& reader)
						{
							for (auto& subMesh : mesh.SubMeshes)
							{
								uint32_t unknown0 = reader.ReadUInt32();
								subMesh.BoundingSphere = ReadSphere(reader);
								subMesh.MaterialIndex = reader.ReadUInt32();
								for (auto& index : subMesh.MaterialUVIndices)
									index = reader.ReadUInt32();

								uint32_t boneIndicesCount = reader.ReadUInt32();
								void* boneIndicesPtr = reader.ReadPtr();
								if (boneIndicesCount > 0 && boneIndicesPtr != nullptr)
								{
									// TODO:
								}

								uint32_t unknown1 = reader.ReadUInt32();
								subMesh.Primitive = static_cast<PrimitiveType>(reader.ReadUInt32());
								uint32_t unknown2 = reader.ReadUInt32();

								uint32_t indexCount = reader.ReadUInt32();
								void* indicesPtr = reader.ReadPtr();
								if (indexCount > 0 && indicesPtr != nullptr)
								{
									subMesh.Indices.resize(indexCount);

									reader.ReadAt(indicesPtr, objBaseAddress, [&subMesh, indexCount](BinaryReader& reader)
									{
										assert(reader.GetEndianness() == Endianness::Little);
										reader.Read(subMesh.Indices.data(), indexCount * sizeof(uint16_t));
									});
								}

								subMesh.BoundingBox = ReadBox(reader);
								uint32_t unknown3 = reader.ReadUInt32();
								uint32_t unknown4 = reader.ReadUInt32();
							}
						});
					}

					mesh.AttributeFlags = reader.ReadUInt32();
					mesh.VertexData.Stride = reader.ReadUInt32();
					mesh.VertexData.VertexCount = reader.ReadUInt32();

					constexpr size_t attributeSize = 28;

					void* vertexAttributePtrs[attributeSize];
					for (auto& attributePtr : vertexAttributePtrs)
						attributePtr = reader.ReadPtr();

					assert(reader.GetEndianness() == Endianness::Little);
					CheckReadVertexData(reader, mesh, VertexAttribute_Position, mesh.VertexData.Positions, vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Normal, mesh.VertexData.Normals, vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_0x3, mesh.VertexData.Attribute_0x3, vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, vertexAttributePtrs, objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, vertexAttributePtrs, objBaseAddress);

					reader.Read(mesh.Name, sizeof(mesh.Name));
				}
			});
		}

		uint32_t materialCount = reader.ReadUInt32();
		void* materialsPtr = reader.ReadPtr();
		if (materialCount > 0 && materialsPtr != nullptr)
		{
			Materials.resize(materialCount);
			reader.ReadAt(materialsPtr, objBaseAddress, [this](BinaryReader& reader)
			{
				for (auto& material : Materials)
				{
					assert(reader.GetEndianness() == Endianness::Little);
					reader.Read(&material, sizeof(Material));
				}
			});
		}
	}

	void ObjSet::Read(BinaryReader& reader)
	{
		uint32_t signature = reader.ReadUInt32();
		uint32_t objectCount = reader.ReadUInt32();
		uint32_t boneCount = reader.ReadUInt32();
		void* objectsPtr = reader.ReadPtr();

		if (objectCount > 0 && objectsPtr != nullptr)
		{
			objects.resize(objectCount);
			reader.ReadAt(objectsPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
				{
					reader.ReadAt(reader.ReadPtr(), [&obj](BinaryReader& reader)
					{
						obj.Read(reader);
					});
				}
			});
		}

		void* objectSkinningsPtr = reader.ReadPtr();
		if (objectCount > 0 && objectSkinningsPtr != nullptr)
		{
			reader.ReadAt(objectSkinningsPtr, [this](BinaryReader& reader)
			{
				// TODO:
			});
		}

		void* objectNamesPtr = reader.ReadPtr();
		if (objectCount > 0 && objectNamesPtr != nullptr)
		{
			reader.ReadAt(objectNamesPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
					obj.Name = reader.ReadStrPtr();
			});
		}

		void* objectIDsPtr = reader.ReadPtr();
		if (objectCount > 0 && objectIDsPtr != nullptr)
		{
			reader.ReadAt(objectIDsPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
					obj.ID = reader.ReadUInt32();
			});
		}

		void* textureIDsPtr = reader.ReadPtr();
		uint32_t textureCount = reader.ReadUInt32();

		if (textureIDsPtr != nullptr && textureCount > 0)
		{
			TextureIDs.resize(textureCount);
			reader.ReadAt(textureIDsPtr, [this](BinaryReader& reader)
			{
				for (auto& textureID : TextureIDs)
					textureID = reader.ReadUInt32();
			});
		}
	}
}

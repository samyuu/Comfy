#include "ObjSet.h"
#include "FileSystem/BinaryReader.h"

namespace FileSystem
{
	static inline vec3 ReadVec3(BinaryReader& reader)
	{
		return vec3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
	}

	static inline Sphere ReadSphere(BinaryReader& reader)
	{
		return { ReadVec3(reader), reader.ReadFloat() };
	}

	static inline Box ReadBox(BinaryReader& reader)
	{
		return { ReadVec3(reader), ReadVec3(reader) };
	}

	template <class T>
	static inline void CheckReadVertexData(BinaryReader& reader, bool attributeFlag, Vector<T>& vector, uint32_t vertexCount, void* pointer, void* baseAddress)
	{
		if (!attributeFlag || pointer == nullptr)
			return;
		
		reader.ReadAt(pointer, baseAddress, [&vector, vertexCount](BinaryReader& reader)
		{
			vector.resize(vertexCount);
			reader.Read(vector.data(), vector.size() * sizeof(T));
		});
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
			Meshes.reserve(meshCount);
			reader.ReadAt(meshesPtr, objBaseAddress, [this, meshCount, objBaseAddress](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < meshCount; i++)
				{
					Meshes.push_back(MakeRef<Mesh>());
					const RefPtr<Mesh>& mesh = Meshes.back();

					uint32_t unknown0 = reader.ReadUInt32();
					mesh->BoundingSphere = ReadSphere(reader);

					uint32_t subMeshCount = reader.ReadUInt32();
					void* subMeshesPtr = reader.ReadPtr();
					if (subMeshCount > 0 && subMeshesPtr != nullptr)
					{
						mesh->SubMeshes.reserve(subMeshCount);
						reader.ReadAt(subMeshesPtr, objBaseAddress, [&mesh, subMeshCount, objBaseAddress](BinaryReader& reader)
						{
							for (uint32_t i = 0; i < subMeshCount; i++)
							{
								mesh->SubMeshes.push_back(MakeRef<SubMesh>());
								const RefPtr<SubMesh>& subMesh = mesh->SubMeshes.back();

								uint32_t unknown0 = reader.ReadUInt32();
								subMesh->BoundingSphere = ReadSphere(reader);
								subMesh->MaterialIndex = reader.ReadUInt32();
								for (auto& index : subMesh->MaterialUVIndices)
									index = reader.ReadUInt32();

								uint32_t boneIndicesCount = reader.ReadUInt32();
								void* boneIndicesPtr = reader.ReadPtr();
								if (boneIndicesCount > 0 && boneIndicesPtr != nullptr)
								{
									// TODO:
								}

								uint32_t unknown1 = reader.ReadUInt32();
								subMesh->Primitive = static_cast<Graphics::PrimitiveType>(reader.ReadUInt32());
								uint32_t unknown2 = reader.ReadUInt32();

								uint32_t indexCount = reader.ReadUInt32();
								void* indicesPtr = reader.ReadPtr();
								if (indexCount > 0 && indicesPtr != nullptr)
								{
									subMesh->Indices.resize(indexCount);

									reader.ReadAt(indicesPtr, objBaseAddress, [&subMesh, indexCount](BinaryReader& reader)
									{
										if (reader.GetEndianness() == Endianness::Little)
										{
											reader.Read(subMesh->Indices.data(), indexCount * sizeof(uint16_t));
										}
										else
										{
											for (uint32_t i = 0; i < indexCount; i++)
												subMesh->Indices[i] = reader.ReadUInt16();
										}
									});
								}

								subMesh->BoundingBox = ReadBox(reader);
								uint32_t unknown3 = reader.ReadUInt32();
								uint32_t unknown4 = reader.ReadUInt32();
							}
						});
					}

					mesh->VertexAttributes.AllBits = reader.ReadUInt32();
					mesh->VertexData.Stride = reader.ReadUInt32();
					uint32_t vertexCount = reader.ReadUInt32();

					constexpr size_t attributeSize = 28;

					void* vertexAttributePtrs[attributeSize];
					for (auto& attributePtr : vertexAttributePtrs)
						attributePtr = reader.ReadPtr();

					assert(reader.GetEndianness() == Endianness::Little);
					CheckReadVertexData(reader, mesh->VertexAttributes.Position, mesh->VertexData.Positions, vertexCount, vertexAttributePtrs[0], objBaseAddress);
					CheckReadVertexData(reader, mesh->VertexAttributes.Normal, mesh->VertexData.Normals, vertexCount, vertexAttributePtrs[1], objBaseAddress);
					CheckReadVertexData(reader, mesh->VertexAttributes.Tangents, mesh->VertexData.Tangents, vertexCount, vertexAttributePtrs[2], objBaseAddress);

					constexpr size_t nameSize = 64;
					mesh->Name = reader.ReadStr(nameSize);
				}
			});
		}

		uint32_t materialCount = reader.ReadUInt32();
		void* materialsPtr = reader.ReadPtr();
		if (materialCount > 0 && materialsPtr != nullptr)
		{
			Materials.reserve(materialCount);
			reader.ReadAt(materialsPtr, [this, materialCount](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < materialCount; i++)
				{
					Materials.push_back(MakeRef<Material>());
					const RefPtr<Material>& material = Materials.back();

					// TODO:
					//material->
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
			objects.reserve(objectCount);
			reader.ReadAt(objectsPtr, [this, objectCount](BinaryReader& reader)
			{
				for (uint32_t i = 0; i < objectCount; i++)
				{
					objects.push_back(MakeRef<Obj>());
					const RefPtr<Obj>& obj = objects.back();

					reader.ReadAt(reader.ReadPtr(), [&obj](BinaryReader& reader)
					{
						obj->Read(reader);
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
				for (RefPtr<Obj>& obj : objects)
					obj->Name = reader.ReadStrPtr();
			});
		}

		void* objectIDsPtr = reader.ReadPtr();
		if (objectCount > 0 && objectIDsPtr != nullptr)
		{
			reader.ReadAt(objectIDsPtr, [this](BinaryReader& reader)
			{
				for (RefPtr<Obj>& obj : objects)
					obj->ID = reader.ReadUInt32();
			});
		}

		void* textureIDsPtr = reader.ReadPtr();
		uint32_t textureCount = reader.ReadUInt32();
	}
}
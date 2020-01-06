#include "ObjSet.h"
#include "FileSystem/BinaryReader.h"

using namespace FileSystem;

namespace Graphics
{
	namespace
	{
		constexpr vec3 TranposeVec3(const vec3& value)
		{
			return vec3(value[2], value[1], value[0]);
		}

		vec3 ReadVec3(BinaryReader& reader)
		{
			return vec3(reader.ReadFloat(), reader.ReadFloat(), reader.ReadFloat());
		}

		// NOTE: Not sure why these are stored differently from the way the vertex positions are
		vec3 ReadTranposeVec3(BinaryReader& reader)
		{
			return TranposeVec3(ReadVec3(reader));
		}

		void ReadMat4(BinaryReader& reader, mat4& output)
		{
			assert(reader.GetEndianness() == Endianness::Little);
			reader.Read(&output, sizeof(output));
		}

		Sphere ReadSphere(BinaryReader& reader)
		{
			return { ReadTranposeVec3(reader), reader.ReadFloat() };
		}

		Box ReadBox(BinaryReader& reader)
		{
			return { ReadTranposeVec3(reader), ReadTranposeVec3(reader) };
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

								uint32_t boneIndexCount = reader.ReadUInt32();
								void* boneIndicesPtr = reader.ReadPtr();
								if (boneIndexCount > 0 && boneIndicesPtr != nullptr)
								{
									subMesh.BoneIndices.resize(boneIndexCount);

									reader.ReadAt(boneIndicesPtr, objBaseAddress, [&subMesh, boneIndexCount](BinaryReader& reader)
									{
										assert(reader.GetEndianness() == Endianness::Little);
										reader.Read(subMesh.BoneIndices.data(), boneIndexCount * sizeof(uint16_t));
									});
								}

								subMesh.UnknownPrePrimitive = reader.ReadUInt32();
								subMesh.Primitive = static_cast<PrimitiveType>(reader.ReadUInt32());
								subMesh.UnknownIndex = reader.ReadUInt32();

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

					constexpr size_t attributeCount = 20;

					void* vertexAttributePtrs[attributeCount];
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

					*reinterpret_cast<uint32_t*>(&mesh.Flags) = reader.ReadUInt32();
					for (int i = 0; i < 7; i++)
						reader.ReadUInt32();

					reader.Read(mesh.Name.data(), sizeof(mesh.Name));
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

		if (void* skeletonsPtr = reader.ReadPtr(); objectCount > 0 && skeletonsPtr != nullptr)
		{
			reader.ReadAt(skeletonsPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
				{
					if (void* skeletonPtr = reader.ReadPtr(); skeletonPtr != nullptr)
					{
						reader.ReadAt(skeletonPtr, [&obj](BinaryReader& reader)
						{
							void* idsPtr = reader.ReadPtr();
							void* transformsPtr = reader.ReadPtr();
							void* namesPtr = reader.ReadPtr();
							void* expressionBlocksPtr = reader.ReadPtr();
							uint32_t count = reader.ReadUInt32();
							void* parentIDsPtr = reader.ReadPtr();

							if (count < 1)
								return;

							auto& bones = obj.Skeleton.Bones;
							bones.resize(count);

							reader.ReadAt(idsPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									bone.ID = reader.ReadUInt32();
							});

							reader.ReadAt(transformsPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									ReadMat4(reader, bone.Transform);
							});

							reader.ReadAt(namesPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									bone.Name = reader.ReadStrPtr();
							});

							reader.ReadAt(expressionBlocksPtr, [&bones](BinaryReader& reader)
							{
								// TODO:
							});

							reader.ReadAt(parentIDsPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									bone.ParentID = reader.ReadUInt32();
							});
						});
					}
				}
			});
		}

		if (void* objectNamesPtr = reader.ReadPtr(); objectCount > 0 && objectNamesPtr != nullptr)
		{
			reader.ReadAt(objectNamesPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
					obj.Name = reader.ReadStrPtr();
			});
		}

		if (void* objectIDsPtr = reader.ReadPtr(); objectCount > 0 && objectIDsPtr != nullptr)
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

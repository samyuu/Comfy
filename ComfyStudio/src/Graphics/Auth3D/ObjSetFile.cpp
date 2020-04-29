#include "ObjSet.h"
#include "IO/BinaryReader.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	namespace
	{
		Sphere ReadSphere(BinaryReader& reader)
		{
			Sphere result;
			result.Center = reader.ReadV3();
			result.Radius = reader.ReadF32();
			return result;
		}

		Box ReadBox(BinaryReader& reader)
		{
			Box result;
			result.Center = reader.ReadV3();
			result.Size = reader.ReadV3();
			return result;
		}

		template <typename FlagsStruct>
		FlagsStruct ReadFlagsStruct32(BinaryReader& reader)
		{
			static_assert(sizeof(FlagsStruct) == sizeof(uint32_t));
			
			const auto uintFlags = reader.ReadU32();
			return *reinterpret_cast<const FlagsStruct*>(&uintFlags);
		}

		template <typename T>
		void CheckReadVertexData(BinaryReader& reader, Mesh& mesh, VertexAttribute_Enum attribute, std::vector<T>& vector, FileAddr* attributePointers, FileAddr baseAddress)
		{
			FileAddr attributePointer = attributePointers[attribute];
			if (attributePointer == FileAddr::NullPtr)
				return;

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
				return;

			reader.ReadAt(attributePointer, baseAddress, [&vector, &mesh](BinaryReader& reader)
			{
				vector.resize(mesh.VertexData.VertexCount);
				reader.ReadBuffer(vector.data(), vector.size() * sizeof(T));
			});
		}
	}

	void Obj::Read(BinaryReader& reader)
	{
		FileAddr objBaseAddress = reader.GetPosition();

		Version = reader.ReadU32();
		ReservedFlags = reader.ReadU32();
		BoundingSphere = ReadSphere(reader);

		uint32_t meshCount = reader.ReadU32();
		FileAddr meshesPtr = reader.ReadPtr();
		if (meshCount > 0 && meshesPtr != FileAddr::NullPtr)
		{
			Meshes.resize(meshCount);
			reader.ReadAt(meshesPtr, objBaseAddress, [this, objBaseAddress](BinaryReader& reader)
			{
				for (auto& mesh : Meshes)
				{
					mesh.ReservedFlags = reader.ReadU32();
					mesh.BoundingSphere = ReadSphere(reader);

					uint32_t subMeshCount = reader.ReadU32();
					FileAddr subMeshesPtr = reader.ReadPtr();
					if (subMeshCount > 0 && subMeshesPtr != FileAddr::NullPtr)
					{
						mesh.SubMeshes.resize(subMeshCount);
						reader.ReadAt(subMeshesPtr, objBaseAddress, [&mesh, objBaseAddress](BinaryReader& reader)
						{
							for (auto& subMesh : mesh.SubMeshes)
							{
								subMesh.ReservedFlags = reader.ReadU32();
								subMesh.BoundingSphere = ReadSphere(reader);
								subMesh.MaterialIndex = reader.ReadU32();
								for (auto& index : subMesh.UVIndices)
									index = reader.ReadU8();

								uint32_t boneIndexCount = reader.ReadU32();
								FileAddr boneIndicesPtr = reader.ReadPtr();
								if (boneIndexCount > 0 && boneIndicesPtr != FileAddr::NullPtr)
								{
									subMesh.BoneIndices.resize(boneIndexCount);

									reader.ReadAt(boneIndicesPtr, objBaseAddress, [&subMesh, boneIndexCount](BinaryReader& reader)
									{
										assert(reader.GetEndianness() == Endianness::Native);
										reader.ReadBuffer(subMesh.BoneIndices.data(), boneIndexCount * sizeof(uint16_t));
									});
								}

								subMesh.BonesPerVertex = reader.ReadU32();
								subMesh.Primitive = static_cast<PrimitiveType>(reader.ReadU32());
								
								auto indexFormat = static_cast<IndexFormat>(reader.ReadU32());

								uint32_t indexCount = reader.ReadU32();
								FileAddr indicesPtr = reader.ReadPtr();
								if (indexCount > 0 && indicesPtr != FileAddr::NullPtr)
								{
									assert(reader.GetEndianness() == Endianness::Native);
									if (indexFormat == IndexFormat::U8)
									{
										uint8_t* data = subMesh.Indices.emplace<std::vector<uint8_t>>(indexCount).data();
										reader.ReadAt(indicesPtr, objBaseAddress, [&](BinaryReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(uint8_t)); });
									}
									else if (indexFormat == IndexFormat::U16)
									{
										uint16_t* data = subMesh.Indices.emplace<std::vector<uint16_t>>(indexCount).data();
										reader.ReadAt(indicesPtr, objBaseAddress, [&](BinaryReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(uint16_t)); });
									}
									else if (indexFormat == IndexFormat::U32)
									{
										uint32_t* data = subMesh.Indices.emplace<std::vector<uint32_t>>(indexCount).data();
										reader.ReadAt(indicesPtr, objBaseAddress, [&](BinaryReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(uint32_t)); });
									}
								}

								subMesh.Flags = ReadFlagsStruct32<SubMesh::SubMeshFlags>(reader);

								std::array<unk32_t, 6> reserved;
								for (auto& value : reserved)
									value = reader.ReadU32();

								unk32_t indexOffset = reader.ReadU32();
							}
						});
					}

					mesh.AttributeFlags = reader.ReadU32();
					mesh.VertexData.Stride = reader.ReadU32();
					mesh.VertexData.VertexCount = reader.ReadU32();

					constexpr size_t attributeCount = 20;

					std::array<FileAddr, attributeCount> vertexAttributePtrs;
					for (auto& attributePtr : vertexAttributePtrs)
						attributePtr = reader.ReadPtr();

					assert(reader.GetEndianness() == Endianness::Native);
					CheckReadVertexData(reader, mesh, VertexAttribute_Position, mesh.VertexData.Positions, vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Normal, mesh.VertexData.Normals, vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_0x3, mesh.VertexData.Reserved0x3, vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, vertexAttributePtrs.data(), objBaseAddress);
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, vertexAttributePtrs.data(), objBaseAddress);

					mesh.Flags = ReadFlagsStruct32<Mesh::MeshFlags>(reader);

					for (auto& reserved : mesh.ReservedData)
						reserved = reader.ReadU32();

					reader.ReadBuffer(mesh.Name.data(), sizeof(mesh.Name));
				}
			});
		}

		uint32_t materialCount = reader.ReadU32();
		FileAddr materialsPtr = reader.ReadPtr();
		if (materialCount > 0 && materialsPtr != FileAddr::NullPtr)
		{
			Materials.resize(materialCount);
			reader.ReadAt(materialsPtr, objBaseAddress, [this](BinaryReader& reader)
			{
				for (auto& material : Materials)
				{
					material.UsedTexturesCount = reader.ReadU32();
					material.UsedTexturesFlags = ReadFlagsStruct32<Material::MaterialUsedTextureFlags>(reader);

					reader.ReadBuffer(material.ShaderType.data(), material.ShaderType.size());
					material.ShaderFlags = ReadFlagsStruct32<Material::MaterialShaderFlags>(reader);

					for (auto& texture : material.Textures)
					{
						texture.SamplerFlags = ReadFlagsStruct32<MaterialTextureData::TextureSamplerFlags>(reader);
						texture.TextureID = TexID(reader.ReadU32());
						texture.TextureFlags = ReadFlagsStruct32<MaterialTextureData::TextureDataFlags>(reader);
						
						reader.ReadBuffer(texture.ExShader.data(), texture.ExShader.size());
						texture.Weight = reader.ReadF32();

						texture.TextureCoordinateMatrix = reader.ReadMat4();
						for (float& reserved : texture.ReservedData)
							reserved = reader.ReadF32();
					}

					material.BlendFlags = ReadFlagsStruct32<Material::MaterialBlendFlags>(reader);

					material.Color.Diffuse = reader.ReadV3();
					material.Color.Transparency = reader.ReadF32();
					material.Color.Ambient = reader.ReadV4();
					material.Color.Specular = reader.ReadV3();
					material.Color.Reflectivity = reader.ReadF32();
					material.Color.Emission = reader.ReadV4();
					material.Color.Shininess = reader.ReadF32();
					material.Color.Intensity = reader.ReadF32();

					material.ReservedSphere = ReadSphere(reader);

					reader.ReadBuffer(material.Name.data(), material.Name.size());
					material.BumpDepth = reader.ReadF32();

					for (float& reserved : material.ReservedData)
						reserved = reader.ReadF32();
				}
			});
		}
	}

	void ObjSet::Read(BinaryReader& reader)
	{
		uint32_t signature = reader.ReadU32();
		uint32_t objectCount = reader.ReadU32();
		uint32_t boneCount = reader.ReadU32();
		FileAddr objectsPtr = reader.ReadPtr();

		if (objectCount > 0 && objectsPtr != FileAddr::NullPtr)
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

		if (FileAddr skeletonsPtr = reader.ReadPtr(); objectCount > 0 && skeletonsPtr != FileAddr::NullPtr)
		{
			reader.ReadAt(skeletonsPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
				{
					if (FileAddr skeletonPtr = reader.ReadPtr(); skeletonPtr != FileAddr::NullPtr)
					{
						reader.ReadAt(skeletonPtr, [&obj](BinaryReader& reader)
						{
							FileAddr idsPtr = reader.ReadPtr();
							FileAddr transformsPtr = reader.ReadPtr();
							FileAddr namesPtr = reader.ReadPtr();
							FileAddr expressionBlocksPtr = reader.ReadPtr();
							uint32_t count = reader.ReadU32();
							FileAddr parentIDsPtr = reader.ReadPtr();

							if (count < 1)
								return;

							auto& bones = obj.Skeleton.emplace().Bones;
							bones.resize(count);

							reader.ReadAt(idsPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									bone.ID = BoneID(reader.ReadU32());
							});

							reader.ReadAt(transformsPtr, [&bones](BinaryReader& reader)
							{
								for (auto& bone : bones)
									bone.Transform = reader.ReadMat4();
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
									bone.ParentID = BoneID(reader.ReadU32());
							});
						});
					}
				}
			});
		}

		if (FileAddr objectNamesPtr = reader.ReadPtr(); objectCount > 0 && objectNamesPtr != FileAddr::NullPtr)
		{
			reader.ReadAt(objectNamesPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
					obj.Name = reader.ReadStrPtr();
			});
		}

		if (FileAddr objectIDsPtr = reader.ReadPtr(); objectCount > 0 && objectIDsPtr != FileAddr::NullPtr)
		{
			reader.ReadAt(objectIDsPtr, [this](BinaryReader& reader)
			{
				for (auto& obj : objects)
					obj.ID = ObjID(reader.ReadU32());
			});
		}

		FileAddr textureIDsPtr = reader.ReadPtr();
		uint32_t textureCount = reader.ReadU32();

		if (textureIDsPtr != FileAddr::NullPtr && textureCount > 0)
		{
			TextureIDs.resize(textureCount);
			reader.ReadAt(textureIDsPtr, [this](BinaryReader& reader)
			{
				for (auto& textureID : TextureIDs)
					textureID = TexID(reader.ReadU32());
			});
		}
	}
}

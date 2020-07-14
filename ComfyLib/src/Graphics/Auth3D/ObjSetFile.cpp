#include "ObjSet.h"
#include "IO/Stream/Manipulator/StreamReader.h"

using namespace Comfy::IO;

namespace Comfy::Graphics
{
	namespace
	{
		Sphere ReadSphere(StreamReader& reader)
		{
			Sphere result;
			result.Center = reader.ReadV3();
			result.Radius = reader.ReadF32();
			return result;
		}

		Box ReadBox(StreamReader& reader)
		{
			Box result;
			result.Center = reader.ReadV3();
			result.Size = reader.ReadV3();
			return result;
		}

		template <typename FlagsStruct>
		FlagsStruct ReadFlagsStruct32(StreamReader& reader)
		{
			static_assert(sizeof(FlagsStruct) == sizeof(u32));
			
			const auto uintFlags = reader.ReadU32();
			return *reinterpret_cast<const FlagsStruct*>(&uintFlags);
		}

		template <typename T>
		void CheckReadVertexData(StreamReader& reader, Mesh& mesh, VertexAttribute_Enum attribute, std::vector<T>& vector, FileAddr* attributePointers)
		{
			FileAddr attributePointer = attributePointers[attribute];
			if (attributePointer == FileAddr::NullPtr)
				return;

			VertexAttributeFlags attributeFlags = (1 << attribute);
			if (!(mesh.AttributeFlags & attributeFlags))
				return;

			reader.ReadAtOffsetAware(attributePointer, [&vector, &mesh](StreamReader& reader)
			{
				vector.resize(mesh.VertexData.VertexCount);
				reader.ReadBuffer(vector.data(), vector.size() * sizeof(T));
			});
		}
	}

	void Obj::Read(StreamReader& reader)
	{
		if (true)
			reader.PushBaseOffset();

		Version = reader.ReadU32();
		ReservedFlags = reader.ReadU32();
		BoundingSphere = ReadSphere(reader);

		const auto meshCount = reader.ReadU32();
		const auto meshesOffset = reader.ReadPtr();
		if (meshCount > 0 && meshesOffset != FileAddr::NullPtr)
		{
			Meshes.reserve(meshCount);
			reader.ReadAtOffsetAware(meshesOffset, [&](StreamReader& reader)
			{
				for (size_t i = 0; i < meshCount; i++)
				{
					auto& mesh = Meshes.emplace_back();
					mesh.ReservedFlags = reader.ReadU32();
					mesh.BoundingSphere = ReadSphere(reader);

					const auto subMeshCount = reader.ReadU32();
					const auto subMeshesOffset = reader.ReadPtr();
					if (subMeshCount > 0 && subMeshesOffset != FileAddr::NullPtr)
					{
						mesh.SubMeshes.resize(subMeshCount);
						reader.ReadAtOffsetAware(subMeshesOffset, [&mesh](StreamReader& reader)
						{
							for (auto& subMesh : mesh.SubMeshes)
							{
								subMesh.ReservedFlags = reader.ReadU32();
								subMesh.BoundingSphere = ReadSphere(reader);
								subMesh.MaterialIndex = reader.ReadU32();
								for (auto& index : subMesh.UVIndices)
									index = reader.ReadU8();

								const auto boneIndexCount = reader.ReadU32();
								const auto boneIndicesOffset = reader.ReadPtr();
								if (boneIndexCount > 0 && boneIndicesOffset != FileAddr::NullPtr)
								{
									subMesh.BoneIndices.resize(boneIndexCount);

									reader.ReadAtOffsetAware(boneIndicesOffset, [&subMesh, boneIndexCount](StreamReader& reader)
									{
										assert(reader.GetEndianness() == Endianness::Native);
										reader.ReadBuffer(subMesh.BoneIndices.data(), boneIndexCount * sizeof(u16));
									});
								}

								subMesh.BonesPerVertex = reader.ReadU32();
								subMesh.Primitive = static_cast<PrimitiveType>(reader.ReadU32());
								
								const auto indexFormat = static_cast<IndexFormat>(reader.ReadU32());

								const auto indexCount = reader.ReadU32();
								const auto indicesOffset = reader.ReadPtr();
								if (indexCount > 0 && indicesOffset != FileAddr::NullPtr)
								{
									assert(reader.GetEndianness() == Endianness::Native);
									if (indexFormat == IndexFormat::U8)
									{
										u8* data = subMesh.Indices.emplace<std::vector<u8>>(indexCount).data();
										reader.ReadAtOffsetAware(indicesOffset, [&](StreamReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(u8)); });
									}
									else if (indexFormat == IndexFormat::U16)
									{
										u16* data = subMesh.Indices.emplace<std::vector<u16>>(indexCount).data();
										reader.ReadAtOffsetAware(indicesOffset, [&](StreamReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(u16)); });
									}
									else if (indexFormat == IndexFormat::U32)
									{
										u32* data = subMesh.Indices.emplace<std::vector<u32>>(indexCount).data();
										reader.ReadAtOffsetAware(indicesOffset, [&](StreamReader& reader) { reader.ReadBuffer(data, indexCount * sizeof(u32)); });
									}
								}

								subMesh.Flags = ReadFlagsStruct32<SubMesh::SubMeshFlags>(reader);

								std::array<unk32_t, 6> reserved;
								for (auto& value : reserved)
									value = reader.ReadU32();

								const auto indexOffset = reader.ReadU32();
							}
						});
					}

					mesh.AttributeFlags = reader.ReadU32();
					mesh.VertexData.Stride = reader.ReadU32();
					mesh.VertexData.VertexCount = reader.ReadU32();

					constexpr size_t attributeCount = 20;

					std::array<FileAddr, attributeCount> vertexAttributeOffsets;
					for (auto& attributeOffset : vertexAttributeOffsets)
						attributeOffset = reader.ReadPtr();

					assert(reader.GetEndianness() == Endianness::Native);
					CheckReadVertexData(reader, mesh, VertexAttribute_Position, mesh.VertexData.Positions, vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_Normal, mesh.VertexData.Normals, vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_Tangent, mesh.VertexData.Tangents, vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate0, mesh.VertexData.TextureCoordinates[0], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate1, mesh.VertexData.TextureCoordinates[1], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate2, mesh.VertexData.TextureCoordinates[2], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_TextureCoordinate3, mesh.VertexData.TextureCoordinates[3], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_Color0, mesh.VertexData.Colors[0], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_Color1, mesh.VertexData.Colors[1], vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneWeight, mesh.VertexData.BoneWeights, vertexAttributeOffsets.data());
					CheckReadVertexData(reader, mesh, VertexAttribute_BoneIndex, mesh.VertexData.BoneIndices, vertexAttributeOffsets.data());

					mesh.Flags = ReadFlagsStruct32<Mesh::MeshFlags>(reader);

					for (auto& reserved : mesh.ReservedData)
						reserved = reader.ReadU32();

					reader.ReadBuffer(mesh.Name.data(), sizeof(mesh.Name));
				}
			});
		}

		const auto materialCount = reader.ReadU32();
		const auto materialsOffset = reader.ReadPtr();
		if (materialCount > 0 && materialsOffset != FileAddr::NullPtr)
		{
			Materials.resize(materialCount);
			reader.ReadAtOffsetAware(materialsOffset, [this](StreamReader& reader)
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

		if (true)
			reader.PopBaseOffset();
	}

	StreamResult ObjSet::Read(StreamReader& reader)
	{
		constexpr u32 signatureLegacy = 0x5062500;
		constexpr u32 signatureModern = 0x5062501;

		const auto signature = reader.ReadU32_LE();
		if (signature != signatureLegacy)
			return StreamResult::BadFormat;

		const auto objectCount = reader.ReadU32();
		const auto boneCount = reader.ReadU32();
		const auto objectsOffset = reader.ReadPtr();

		if (objectCount > 0)
		{
			if (!reader.IsValidPointer(objectsOffset))
				return StreamResult::BadPointer;

			objects.resize(objectCount);
			reader.ReadAtOffsetAware(objectsOffset, [this](StreamReader& reader)
			{
				for (auto& obj : objects)
				{
					reader.ReadAtOffsetAware(reader.ReadPtr(), [&obj](StreamReader& reader)
					{
						obj.Read(reader);
					});
				}
			});
		}

		const auto skeletonsOffset = reader.ReadPtr();
		if (objectCount > 0)
		{
			if (!reader.IsValidPointer(skeletonsOffset))
				return StreamResult::BadPointer;

			reader.ReadAtOffsetAware(skeletonsOffset, [this](StreamReader& reader)
			{
				for (auto& obj : objects)
				{
					const auto skeletonOffset = reader.ReadPtr();
					if (skeletonOffset != FileAddr::NullPtr)
					{
						reader.ReadAtOffsetAware(skeletonOffset, [&obj](StreamReader& reader)
						{
							const auto idsOffset = reader.ReadPtr();
							const auto transformsOffset = reader.ReadPtr();
							const auto namesOffset = reader.ReadPtr();
							const auto expressionBlocksOffset = reader.ReadPtr();
							const auto boneCount = reader.ReadU32();
							const auto parentIDsOffset = reader.ReadPtr();

							if (boneCount < 1)
								return;

							auto& bones = obj.Skeleton.emplace().Bones;
							bones.resize(boneCount);

							reader.ReadAtOffsetAware(idsOffset, [&bones](StreamReader& reader)
							{
								for (auto& bone : bones)
									bone.ID = BoneID(reader.ReadU32());
							});

							reader.ReadAtOffsetAware(transformsOffset, [&bones](StreamReader& reader)
							{
								for (auto& bone : bones)
									bone.Transform = reader.ReadMat4();
							});

							reader.ReadAtOffsetAware(namesOffset, [&bones](StreamReader& reader)
							{
								for (auto& bone : bones)
									bone.Name = reader.ReadStrPtrOffsetAware();
							});

							reader.ReadAtOffsetAware(expressionBlocksOffset, [&bones](StreamReader& reader)
							{
								// TODO:
							});

							reader.ReadAtOffsetAware(parentIDsOffset, [&bones](StreamReader& reader)
							{
								for (auto& bone : bones)
									bone.ParentID = BoneID(reader.ReadU32());
							});
						});
					}
				}
			});
		}

		const auto objectNamesOffset = reader.ReadPtr();
		if (objectCount > 0)
		{
			if (!reader.IsValidPointer(objectNamesOffset))
				return StreamResult::BadPointer;

			reader.ReadAtOffsetAware(objectNamesOffset, [this](StreamReader& reader)
			{
				for (auto& obj : objects)
					obj.Name = reader.ReadStrPtrOffsetAware();
			});
		}

		const auto objectIDsOffset = reader.ReadPtr();
		if (objectCount > 0)
		{
			if (!reader.IsValidPointer(objectIDsOffset))
				return StreamResult::BadPointer;

			reader.ReadAtOffsetAware(objectIDsOffset, [this](StreamReader& reader)
			{
				for (auto& obj : objects)
					obj.ID = ObjID(reader.ReadU32());
			});
		}

		const auto textureIDsOffset = reader.ReadPtr();
		const auto textureCount = reader.ReadU32();

		if (textureCount > 0)
		{
			if (!reader.IsValidPointer(textureIDsOffset))
				return StreamResult::BadPointer;

			TextureIDs.reserve(textureCount);
			reader.ReadAtOffsetAware(textureIDsOffset, [&](StreamReader& reader)
			{
				for (size_t i = 0; i < textureCount; i++)
					TextureIDs.push_back(TexID(reader.ReadU32()));
			});
		}

		return StreamResult::Success;
	}
}

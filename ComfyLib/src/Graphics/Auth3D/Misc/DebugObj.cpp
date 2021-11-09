#include "DebugObj.h"
#include <unordered_map>
#include <optional>

namespace Comfy::Graphics
{
	namespace
	{
		template <typename IndexType>
		using PositionIndicesPair = std::pair<std::vector<vec3>, std::vector<IndexType>>;

		template <typename IndexType>
		std::array<std::optional<PositionIndicesPair<IndexType>>, MaxSphereMeshDetailLevel + 1> CachedIcosahedronMeshes;

		template <typename T>
		void GenerateDebugObj(std::string_view name, Obj& obj, const Sphere& boundingSphere, const Box& boundingBox, const vec4& color, T meshGeneratorFunc)
		{
			obj.Name.resize(name.size() + std::strlen("::Obj") + 1);
			sprintf_s(obj.Name.data(), obj.Name.size(), "%.*s::Obj", static_cast<int>(name.size()), name.data());

			obj.ID = ObjID('DBG');
			obj.BoundingSphere = boundingSphere;

			obj.Meshes.push_back({});
			Mesh& mesh = obj.Meshes.back();

			mesh.SubMeshes.push_back({});
			SubMesh& subMesh = mesh.SubMeshes.back();
			{
				subMesh.ReservedFlags = {};
				subMesh.BoundingSphere = boundingSphere;
				subMesh.BoundingBox.emplace(boundingBox);

				subMesh.MaterialIndex = 0;
				subMesh.UVIndices = {};

				subMesh.BonesPerVertex = 0;
				subMesh.Primitive = PrimitiveType::Triangles;

				subMesh.Flags = {};

				// TODO: Optionally generate normals but not needed for now
				// bool generateNormals = ...;

				meshGeneratorFunc(mesh, subMesh);

				mesh.BoundingSphere = boundingSphere;
				mesh.AttributeFlags =
					(mesh.VertexData.Positions.empty() ? 0 : VertexAttributeFlags_Position) |
					(mesh.VertexData.Normals.empty() ? 0 : VertexAttributeFlags_Normal) |
					(mesh.VertexData.Tangents.empty() ? 0 : VertexAttributeFlags_Tangent) |
					(mesh.VertexData.TextureCoordinates.front().empty() ? 0 : VertexAttributeFlags_TextureCoordinate0) |
					(mesh.VertexData.Colors.front().empty() ? 0 : VertexAttributeFlags_Color0);

				mesh.Flags = {};
				sprintf_s(mesh.Name.data(), mesh.Name.size(), "%.*s::Mesh", static_cast<int>(name.size()), name.data());

				mesh.VertexData.Stride = static_cast<u32>(mesh.VertexData.Positions.size() * sizeof(vec3));
				mesh.VertexData.VertexCount = static_cast<u32>(mesh.VertexData.Positions.size());
			}

			Material material = {};
			{
				material.ShaderType = Material::ShaderIdentifiers::Blinn;
				material.ShaderFlags.LambertShading = false;
				material.ShaderFlags.PhongShading = false;
				material.BlendFlags.AlphaMaterial = (color.a < 1.0f);
				material.BlendFlags.SrcBlendFactor = BlendFactor::SrcAlpha;
				material.BlendFlags.DstBlendFactor = BlendFactor::InverseSrcAlpha;
				material.BlendFlags.DoubleSided = true;
				for (auto& texture : material.Textures)
					texture.TextureID = TexID::Invalid;
				material.Color.Diffuse = vec3(color);
				material.Color.Transparency = color.a;
				material.Color.Ambient = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				material.Color.Specular = vec3(1.0f, 1.0f, 1.0f);
				material.Color.Reflectivity = 1.0f;
				material.Color.Emission = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				material.Color.Shininess = 50.0f;
				material.Color.Intensity = 0.0f;
				sprintf_s(material.Name.data(), material.Name.size(), "%.*s::Material", static_cast<int>(name.size()), name.data());
				material.BumpDepth = 1.0f;
			}
			obj.Materials.push_back(material);
		}

		template <typename IndexType>
		void GenerateUnitBoxMesh(std::vector<vec3>& outPositions, std::vector<IndexType>& outIndices)
		{
			outPositions =
			{
				{ -1.0f, +1.0f, -1.0f },
				{ +1.0f, +1.0f, -1.0f },
				{ +1.0f, +1.0f, +1.0f },
				{ -1.0f, +1.0f, +1.0f },
				{ -1.0f, -1.0f, -1.0f },
				{ +1.0f, -1.0f, -1.0f },
				{ +1.0f, -1.0f, +1.0f },
				{ -1.0f, -1.0f, +1.0f },
			};

			outIndices =
			{
				3, 1, 0,
				2, 1, 3,

				0, 5, 4,
				1, 5, 0,

				3, 4, 7,
				0, 4, 3,

				1, 6, 5,
				2, 6, 1,

				2, 7, 6,
				3, 7, 2,

				6, 4, 5,
				7, 4, 6,
			};
		}

		// NOTE: Based on http://blog.andreaskahler.com/2009/06/creating-icosphere-mesh-in-code.html
		template <typename IndexType>
		void GenerateUnitIcosahedronMesh(std::vector<vec3>& outPositions, std::vector<IndexType>& outIndices, int detailLevel)
		{
			if (detailLevel < MinSphereMeshDetailLevel || detailLevel > static_cast<int>(CachedIcosahedronMeshes<IndexType>.size()) - 1)
				return;

			auto& cachedMesh = CachedIcosahedronMeshes<IndexType>[detailLevel];
			const bool isCached = cachedMesh.has_value();

			if (!isCached)
				cachedMesh = PositionIndicesPair<IndexType> {};

			auto&[positions, indices] = cachedMesh.value();

			if (!isCached)
			{
				// NOTE: Create 12 vertices of a icosahedron
				const float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;
				positions =
				{
					glm::normalize(vec3(-1.0f, +t, +0.0f)),
					glm::normalize(vec3(+1.0f, +t, +0.0f)),
					glm::normalize(vec3(-1.0f, -t, +0.0f)),
					glm::normalize(vec3(+1.0f, -t, +0.0f)),

					glm::normalize(vec3(+0.0f, -1.0f, +t)),
					glm::normalize(vec3(+0.0f, +1.0f, +t)),
					glm::normalize(vec3(+0.0f, -1.0f, -t)),
					glm::normalize(vec3(+0.0f, +1.0f, -t)),

					glm::normalize(vec3(+t, +0.0f, -1.0f)),
					glm::normalize(vec3(+t, +0.0f, +1.0f)),
					glm::normalize(vec3(-t, +0.0f, -1.0f)),
					glm::normalize(vec3(-t, +0.0f, +1.0f)),
				};

				// NOTE: Create 20 triangles of the icosahedron
				std::vector<std::array<IndexType, 3>> faces =
				{
					// NOTE: 5 faces around point 0
					{  0, 11,  5 },
					{  0,  5,  1 },
					{  0,  1,  7 },
					{  0,  7, 10 },
					{  0, 10, 11 },

					// NOTE: 5 adjacent faces
					{  1,  5,  9 },
					{  5, 11,  4 },
					{ 11, 10,  2 },
					{ 10,  7,  6 },
					{  7,  1,  8 },

					// NOTE: 5 faces around point 3
					{  3,  9,  4 },
					{  3,  4,  2 },
					{  3,  2,  6 },
					{  3,  6,  8 },
					{  3,  8,  9 },

					// NOTE: 5 adjacent faces
					{  4,  9,  5 },
					{  2,  4, 11 },
					{  6,  2, 10 },
					{  8,  6,  7 },
					{  9,  8,  1 },
				};

				std::unordered_map<i64, IndexType> middlePointIndexCache;

				auto addGetMiddlePointIndex = [&middlePointIndexCache, &positions](IndexType indexA, IndexType indexB)
				{
					bool firstIsSmaller = indexA < indexB;

					i64 smallerIndex = static_cast<i64>(firstIsSmaller ? indexA : indexB);
					i64 greaterIndex = static_cast<i64>(firstIsSmaller ? indexB : indexA);

					i64 hashKey = (smallerIndex << 32) + greaterIndex;

					auto cachedIndex = middlePointIndexCache.find(hashKey);
					if (cachedIndex != middlePointIndexCache.end())
						return cachedIndex->second;

					positions.push_back(glm::normalize((positions[indexA] + positions[indexB]) / 2.0f));

					IndexType index = static_cast<IndexType>(positions.size()) - 1;
					middlePointIndexCache.insert(std::make_pair(hashKey, index));
					return index;
				};

				std::vector<std::array<IndexType, 3>> refinedFaces;

				for (int i = 0; i < detailLevel; i++)
				{
					refinedFaces.clear();
					refinedFaces.reserve(faces.size() * 4);

					for (auto& face : faces)
					{
						std::array middlePointIndices =
						{
							addGetMiddlePointIndex(face[0], face[1]),
							addGetMiddlePointIndex(face[1], face[2]),
							addGetMiddlePointIndex(face[2], face[0]),
						};

						refinedFaces.push_back({ face[0], middlePointIndices[0], middlePointIndices[2] });
						refinedFaces.push_back({ face[1], middlePointIndices[1], middlePointIndices[0] });
						refinedFaces.push_back({ face[2], middlePointIndices[2], middlePointIndices[1] });
						refinedFaces.push_back({ middlePointIndices[0], middlePointIndices[1], middlePointIndices[2] });
					}

					faces = refinedFaces;
				}

				for (auto& triangle : faces)
				{
					indices.push_back(triangle[0]);
					indices.push_back(triangle[1]);
					indices.push_back(triangle[2]);
				}
			}

			outPositions = positions;
			outIndices = indices;
		}
	}

	int GetSphereMeshDetailLevelForRadius(const Sphere& sphere)
	{
		constexpr std::array<float, MaxSphereMeshDetailLevel> sphereMeshRadiusDetailLevels =
		{
			0.035f,
			0.100f,
			0.200f,
			1.000f,
			500.000f,
			5000.000f,
		};

		for (int i = 0; i < sphereMeshRadiusDetailLevels.size(); i++)
		{
			if (sphere.Radius <= sphereMeshRadiusDetailLevels[i])
				return i;
		}

		return MaxSphereMeshDetailLevel;
	}

	std::unique_ptr<Obj> GenerateDebugBoxObj(const Box& box, const vec4& color)
	{
		auto obj = std::make_unique<Obj>();
		{
			const f32 maxSize = Max(Max(box.Size.x, box.Size.y), box.Size.z);
			const Sphere sphere = { box.Center, maxSize };

			GenerateDebugObj("DebugBox", *obj, sphere, box, color, [&](Mesh& mesh, SubMesh& subMesh)
			{
				GenerateUnitBoxMesh(mesh.VertexData.Positions, subMesh.Indices.emplace<std::vector<u8>>());

				for (auto& position : mesh.VertexData.Positions)
				{
					position *= box.Size;
					position += box.Center;
				}
			});
		}
		return obj;
	}

	std::unique_ptr<Obj> GenerateDebugSphereObj(const Sphere& sphere, const vec4& color, int detailLevel)
	{
		if (detailLevel < MinSphereMeshDetailLevel)
			detailLevel = GetSphereMeshDetailLevelForRadius(sphere);

		auto obj = std::make_unique<Obj>();
		{
			const Box box = { sphere.Center, vec3(sphere.Radius) };
			GenerateDebugObj("DebugSphere", *obj, sphere, box, color, [&](Mesh& mesh, SubMesh& subMesh)
			{
				GenerateUnitIcosahedronMesh(mesh.VertexData.Positions, subMesh.Indices.emplace<std::vector<u16>>(), detailLevel);

				for (auto& position : mesh.VertexData.Positions)
				{
					position *= sphere.Radius;
					position += sphere.Center;
				}
			});
		}
		return obj;
	}

	std::unique_ptr<Obj> GenerateMaterialTestSphereObj()
	{
		constexpr Sphere boundingSphere = { vec3(0.0f), 1.0f };
		const int detailLevel = GetSphereMeshDetailLevelForRadius(boundingSphere);

		auto obj = std::make_unique<Obj>();
		{
			Box box = { boundingSphere.Center, vec3(boundingSphere.Radius) };
			GenerateDebugObj("MaterialTestSphere", *obj, boundingSphere, box, vec4(1.0f, 0.0f, 1.0f, 1.0f), [&](Mesh& mesh, SubMesh& subMesh)
			{
				GenerateUnitIcosahedronMesh(mesh.VertexData.Positions, subMesh.Indices.emplace<std::vector<u16>>(), detailLevel);

				mesh.VertexData.VertexCount = static_cast<u32>(mesh.VertexData.Positions.size());

				mesh.VertexData.Normals.resize(mesh.VertexData.VertexCount);
				mesh.VertexData.Tangents.resize(mesh.VertexData.VertexCount);
				mesh.VertexData.TextureCoordinates[0].resize(mesh.VertexData.VertexCount);
				mesh.VertexData.Colors[0].resize(mesh.VertexData.VertexCount);

				const mat4 normalToTangent = glm::rotate(mat4(1.0f), glm::radians(90.0f), vec3(1.0f, 0.0f, 0.0f));

				for (size_t i = 0; i < mesh.VertexData.VertexCount; i++)
				{
					vec3& normal = mesh.VertexData.Normals[i];
					normal = glm::normalize(mesh.VertexData.Positions[i]);

					// HACK: Quick bodge
					mesh.VertexData.Tangents[i] = vec4(
						glm::normalize(vec3(normalToTangent * vec4(normal, 0.0f))), 
						1.0f);

					mesh.VertexData.TextureCoordinates[0][i] = vec2(
						glm::asin(normal.x) / glm::pi<float>() + 0.5f,
						glm::asin(normal.y) / glm::pi<float>() + 0.5f);

					mesh.VertexData.Colors[0][i] = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				}
			});
		}
		return obj;
	}
}

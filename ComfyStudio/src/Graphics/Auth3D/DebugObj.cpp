#include "DebugObj.h"
#include <unordered_map>
#include <optional>

namespace Comfy::Graphics
{
	namespace
	{
		using PositionIndicesPair = std::pair<std::vector<vec3>, std::vector<uint16_t>>;

		std::array<std::optional<PositionIndicesPair>, MaxSphereMeshDetailLevel + 1> CachedIcosahedronMeshes;

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
				subMesh.BoundingBox = boundingBox;
				subMesh.BoundingSphere = boundingSphere;
				subMesh.MaterialIndex = 0;
				subMesh.Primitive = PrimitiveType::Triangles;

				// TODO: Optionally generate normals but not needed for now
				// bool generateNormals = ...;

				meshGeneratorFunc(mesh, subMesh);

				mesh.BoundingSphere = boundingSphere;
				mesh.AttributeFlags = (mesh.VertexData.Positions.empty() ? 0 : VertexAttributeFlags_Position) | (mesh.VertexData.Normals.empty() ? 0 : VertexAttributeFlags_Normal);

				mesh.Flags = {};
				sprintf_s(mesh.Name.data(), mesh.Name.size(), "%.*s::Mesh", static_cast<int>(name.size()), name.data());

				mesh.VertexData.Stride = static_cast<uint32_t>(mesh.VertexData.Positions.size() * sizeof(vec3));
				mesh.VertexData.VertexCount = static_cast<uint32_t>(mesh.VertexData.Positions.size());
			}

			Material material = {};
			{
				material.MaterialType = Material::Identifiers.BLINN;
				material.ShaderFlags.LambertShading = false;
				material.ShaderFlags.PhongShading = false;
				material.BlendFlags.EnableBlend = (color.a < 1.0f);
				material.BlendFlags.SrcBlendFactor = BlendFactor_SRC_ALPHA;
				material.BlendFlags.DstBlendFactor = BlendFactor_ISRC_ALPHA;
				material.BlendFlags.DoubleSidedness = DoubleSidedness_1_FaceLight;
				material.IterateTextures([](auto& texture) { texture->TextureID = TxpID::Invalid; });
				material.DiffuseColor = vec3(color);
				material.Transparency = color.a;
				material.AmbientColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				material.SpecularColor = vec3(1.0f, 1.0f, 1.0f);
				material.Reflectivity = 1.0f;
				material.EmissionColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
				material.Shininess = 50.0f;
				material.Intensity = 0.0f;
				sprintf_s(material.Name.data(), material.Name.size(), "%.*s::Material", static_cast<int>(name.size()), name.data());
				material.BumpDepth = 1.0f;
			}
			obj.Materials.push_back(material);
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

	void GenerateUnitBoxMesh(std::vector<vec3>& outPositions, std::vector<uint16_t>& outIndices)
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
	void GenerateUnitIcosahedronMesh(std::vector<vec3>& outPositions, std::vector<uint16_t>& outIndices, int detailLevel)
	{
		if (detailLevel < MinSphereMeshDetailLevel || detailLevel > static_cast<int>(CachedIcosahedronMeshes.size()) - 1)
			return;

		auto& cachedMesh = CachedIcosahedronMeshes[detailLevel];
		const bool isCached = cachedMesh.has_value();

		if (!isCached)
			cachedMesh = PositionIndicesPair {};

		auto&[positions, indices] = cachedMesh.value();

		if (!isCached)
		{
			// NOTE: Create 12 vertices of a icosahedron
			const float t = (1.0f + glm::sqrt(5.0f)) / 2.0f;
			positions = 
			{
				normalize(vec3(-1.0f, +t, +0.0f)),
				normalize(vec3(+1.0f, +t, +0.0f)),
				normalize(vec3(-1.0f, -t, +0.0f)),
				normalize(vec3(+1.0f, -t, +0.0f)),

				normalize(vec3(+0.0f, -1.0f, +t)),
				normalize(vec3(+0.0f, +1.0f, +t)),
				normalize(vec3(+0.0f, -1.0f, -t)),
				normalize(vec3(+0.0f, +1.0f, -t)),

				normalize(vec3(+t, +0.0f, -1.0f)),
				normalize(vec3(+t, +0.0f, +1.0f)),
				normalize(vec3(-t, +0.0f, -1.0f)),
				normalize(vec3(-t, +0.0f, +1.0f)),
			};

			// NOTE: Create 20 triangles of the icosahedron
			std::vector<std::array<uint16_t, 3>> faces =
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

			std::unordered_map<int64_t, uint16_t> middlePointIndexCache;

			auto addGetMiddlePointIndex = [&middlePointIndexCache, &positions](uint16_t indexA, uint16_t indexB)
			{
				bool firstIsSmaller = indexA < indexB;

				int64_t smallerIndex = static_cast<int64_t>(firstIsSmaller ? indexA : indexB);
				int64_t greaterIndex = static_cast<int64_t>(firstIsSmaller ? indexB : indexA);

				int64_t hashKey = (smallerIndex << 32) + greaterIndex;

				auto cachedIndex = middlePointIndexCache.find(hashKey);
				if (cachedIndex != middlePointIndexCache.end())
					return cachedIndex->second;

				positions.push_back(normalize((positions[indexA] + positions[indexB]) / 2.0f));

				uint16_t index = static_cast<uint16_t>(positions.size()) - 1;
				middlePointIndexCache.insert(std::make_pair(hashKey, index));
				return index;
			};

			std::vector<std::array<uint16_t, 3>> refinedFaces;

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

	UniquePtr<Obj> GenerateUploadDebugBoxObj(const Box& box, const vec4& color)
	{
		auto obj = MakeUnique<Obj>();
		{
			float maxSize = std::max({ box.Size.x, box.Size.y, box.Size.z });
			Sphere sphere = { box.Center, maxSize };

			GenerateDebugObj("DebugBox", *obj, sphere, box, color, [&](Mesh& mesh, SubMesh& subMesh)
			{
				GenerateUnitBoxMesh(mesh.VertexData.Positions, subMesh.Indices);

				for (auto& position : mesh.VertexData.Positions)
				{
					position *= box.Size;
					position += box.Center;
				}
			});
		}
		obj->Upload();
		return obj;
	}

	UniquePtr<Obj> GenerateUploadDebugSphereObj(const Sphere& sphere, const vec4& color, int detailLevel)
	{
		if (detailLevel < MinSphereMeshDetailLevel)
			detailLevel = GetSphereMeshDetailLevelForRadius(sphere);

		auto obj = MakeUnique<Obj>();
		{
			Box box = { sphere.Center, vec3(sphere.Radius) };
			GenerateDebugObj("DebugSphere", *obj, sphere, box, color, [&](Mesh& mesh, SubMesh& subMesh)
			{
				GenerateUnitIcosahedronMesh(mesh.VertexData.Positions, subMesh.Indices, detailLevel);

				for (auto& position : mesh.VertexData.Positions)
				{
					position *= sphere.Radius;
					position += sphere.Center;
				}
			});
		}
		obj->Upload();
		return obj;
	}
}

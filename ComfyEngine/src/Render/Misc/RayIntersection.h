#pragma once
#include "Render/Core/Camera.h"
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/BoundingTypes.h"
#include "Graphics/Auth3D/ObjSet.h"
#include <glm/gtx/intersect.hpp>

namespace Comfy::Render
{
	inline bool RayIntersectsTriangle(const vec3& viewPoint, const vec3& ray, const vec3* trianglePoints, float& outIntersectionDistance)
	{
		vec2 baryPosition;
		return glm::intersectRayTriangle(viewPoint, ray, trianglePoints[0], trianglePoints[1], trianglePoints[2], baryPosition, outIntersectionDistance);
	}

	inline bool RayIntersectsTriangleSingleSided(const vec3& viewPoint, const vec3& ray, const vec3* trianglePoints, float& outIntersectionDistance)
	{
		const auto normal = glm::normalize(glm::cross(trianglePoints[1] - trianglePoints[0], trianglePoints[2] - trianglePoints[0]));
		if (glm::dot(ray, normal) > 0.0f)
			return false;

		return RayIntersectsTriangle(viewPoint, ray, trianglePoints, outIntersectionDistance);
	}

	inline bool RayIntersectsSphere(const vec3& viewPoint, const vec3& ray, const Graphics::Sphere& sphere, float& outIntesectionDistance)
	{
		return glm::intersectRaySphere(viewPoint, ray, sphere.Center, (sphere.Radius * sphere.Radius), outIntesectionDistance);
	}

	struct RayObjIntersectionResult
	{
		float Distance;
		const Graphics::Mesh* Mesh;
		const Graphics::SubMesh* SubMesh;
	};

	template <typename IndexType, typename Func>
	void ForEachIndexedTriangle(const std::vector<vec3>& vertexPositions, Graphics::PrimitiveType primitiveType, const std::vector<IndexType>& indices, Func perTriangleFunc)
	{
		if (primitiveType == Graphics::PrimitiveType::TriangleStrip)
		{
			for (size_t i = 0; i < indices.size() - 2; i += 1)
			{
				auto triangle = (i % 2 == 0) ?
					std::array<vec3, 3> { vertexPositions[indices[i + 0]], vertexPositions[indices[i + 1]], vertexPositions[indices[i + 2]], } :
					std::array<vec3, 3> { vertexPositions[indices[i + 2]], vertexPositions[indices[i + 1]], vertexPositions[indices[i + 0]], };

				perTriangleFunc(triangle);
			}
		}
		else if (primitiveType == Graphics::PrimitiveType::Triangles)
		{
			for (size_t i = 0; i < indices.size() - 2; i += 3)
			{
				auto triangle = std::array<vec3, 3> { vertexPositions[indices[i + 0]], vertexPositions[indices[i + 1]], vertexPositions[indices[i + 2]], };
				perTriangleFunc(triangle);
			}
		}
	}

	template <typename Func>
	void ForEachSubMeshTriangle(const Graphics::Mesh& mesh, const Graphics::SubMesh& subMesh, Func perTriangleFunc)
	{
		if (subMesh.Primitive != Graphics::PrimitiveType::Triangles && subMesh.Primitive != Graphics::PrimitiveType::TriangleStrip)
			return;

		// NOTE: Check in order of likeliness
		if (auto indices = subMesh.GetIndicesU16(); indices != nullptr)
			ForEachIndexedTriangle(mesh.VertexData.Positions, subMesh.Primitive, *indices, perTriangleFunc);
		else if (auto indices = subMesh.GetIndicesU32(); indices != nullptr)
			ForEachIndexedTriangle(mesh.VertexData.Positions, subMesh.Primitive, *indices, perTriangleFunc);
		else if (auto indices = subMesh.GetIndicesU8(); indices != nullptr)
			ForEachIndexedTriangle(mesh.VertexData.Positions, subMesh.Primitive, *indices, perTriangleFunc);
	}

	inline RayObjIntersectionResult RayIntersectsObj(const vec3& viewPoint, const vec3& ray, const Graphics::Obj& obj, const Graphics::Transform& transform)
	{
		float intersectionDistance = 0.0f;
		if (!RayIntersectsSphere(viewPoint, ray, obj.BoundingSphere * transform, intersectionDistance))
			return RayObjIntersectionResult { 0.0f, nullptr, nullptr };

		// TODO: Can all of the triangle transform multiplication be avoided by inverse transforming the ray / viewpoint instead (?)
		const mat4 transformMatrix = transform.CalculateMatrix();

		float closestDistance = std::numeric_limits<float>::max();
		const Graphics::Mesh* closestMesh = nullptr;
		const Graphics::SubMesh* closestSubMesh = nullptr;

		for (const auto& mesh : obj.Meshes)
		{
			if (!RayIntersectsSphere(viewPoint, ray, mesh.BoundingSphere * transform, intersectionDistance))
				continue;

			for (const auto& subMesh : mesh.SubMeshes)
			{
				if (!RayIntersectsSphere(viewPoint, ray, subMesh.BoundingSphere * transform, intersectionDistance))
					continue;

				const auto& material = IndexOrNull(subMesh.MaterialIndex, obj.Materials);
				auto triangleIntersectionFunc = (material != nullptr && material->BlendFlags.DoubleSided) ? RayIntersectsTriangle : RayIntersectsTriangleSingleSided;

				// BUG: It seems something here still isn't quite right, observed when trying to ray pick the character hair backside at low angles
				ForEachSubMeshTriangle(mesh, subMesh, [&](auto& triangle)
				{
					for (size_t i = 0; i < std::size(triangle); i++)
						triangle[i] = transformMatrix * vec4(triangle[i], 1.0f);

					if (!triangleIntersectionFunc(viewPoint, ray, triangle.data(), intersectionDistance))
						return;

					if (intersectionDistance < closestDistance)
					{
						closestDistance = intersectionDistance;
						closestSubMesh = &subMesh;
						closestMesh = &mesh;
					}
				});
			}
		}

		return RayObjIntersectionResult { closestDistance, closestMesh, closestSubMesh };
	}
}

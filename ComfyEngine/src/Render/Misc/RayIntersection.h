#pragma once
#include "Render/Core/Camera.h"
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/BoundingTypes.h"
#include "Graphics/Auth3D/ObjSet.h"
#include <glm/gtx/intersect.hpp>

namespace Comfy::Render
{
	inline bool RayIntersectsTriangle(const Graphics::Ray& ray, const vec3* trianglePoints, float& outIntersectionDistance)
	{
		vec2 baryPosition;
		return glm::intersectRayTriangle(ray.Origin, ray.Direction, trianglePoints[0], trianglePoints[1], trianglePoints[2], baryPosition, outIntersectionDistance);
	}

	inline bool RayIntersectsTriangleSingleSided(const Graphics::Ray& ray, const vec3* trianglePoints, float& outIntersectionDistance)
	{
		const auto normal = glm::normalize(glm::cross(trianglePoints[1] - trianglePoints[0], trianglePoints[2] - trianglePoints[0]));
		if (glm::dot(ray.Direction, normal) > 0.0f)
			return false;

		return RayIntersectsTriangle(ray, trianglePoints, outIntersectionDistance);
	}

	inline bool RayIntersectsSphere(const Graphics::Ray& ray, const Graphics::Sphere& sphere, float& outIntesectionDistance)
	{
		return glm::intersectRaySphere(ray.Origin, ray.Direction, sphere.Center, (sphere.Radius * sphere.Radius), outIntesectionDistance);
	}

	struct RayObjIntersectionResult
	{
		float Distance;
		const Graphics::Mesh* Mesh;
		const Graphics::SubMesh* SubMesh;
	};

	template <typename AttributeType, typename IndexType, typename Func>
	void ForEachIndexedTriangle(const std::vector<AttributeType>& vertices, Graphics::PrimitiveType primitiveType, const std::vector<IndexType>& indices, Func perTriangleFunc)
	{
#if 0 
		// TODO: Safety bounds check, should probably only be done once on load (?)
		if (std::any_of(indices.begin(), indices.end(), [&](auto index) { return !InBounds(index, vertices); }))
			return;
#else 
		// NOTE: Otherwise specifically check for empty vertices as is the case for a few rare models
		if (vertices.empty())
			return;
#endif

		if (primitiveType == Graphics::PrimitiveType::TriangleStrip)
		{
			for (size_t i = 0; i < indices.size() - 2; i += 1)
			{
				auto triangle = (i % 2 == 0) ?
					std::array { vertices[indices[i + 0]], vertices[indices[i + 1]], vertices[indices[i + 2]], } :
					std::array { vertices[indices[i + 2]], vertices[indices[i + 1]], vertices[indices[i + 0]], };

				perTriangleFunc(triangle);
			}
		}
		else if (primitiveType == Graphics::PrimitiveType::Triangles)
		{
			for (size_t i = 0; i < indices.size() - 2; i += 3)
			{
				auto triangle = std::array { vertices[indices[i + 0]], vertices[indices[i + 1]], vertices[indices[i + 2]], };
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

	inline RayObjIntersectionResult RayIntersectsObj(const Graphics::Ray& ray, float nearPlane, const Graphics::Obj& obj, const Graphics::Transform& transform)
	{
		const mat4 inverseTransform = glm::inverse(transform.CalculateMatrix());
		const auto inverseRay = Graphics::Ray { (inverseTransform * vec4(ray.Origin, 1.0f)), glm::normalize(inverseTransform * vec4(ray.Direction, 0.0f)) };

		float intersectionDistance = 0.0f;
		if (!RayIntersectsSphere(inverseRay, obj.BoundingSphere, intersectionDistance))
			return RayObjIntersectionResult { 0.0f, nullptr, nullptr };

		float closestDistance = std::numeric_limits<float>::max();
		const Graphics::Mesh* closestMesh = nullptr;
		const Graphics::SubMesh* closestSubMesh = nullptr;

		for (const auto& mesh : obj.Meshes)
		{
			if (!RayIntersectsSphere(inverseRay, mesh.BoundingSphere, intersectionDistance))
				continue;

			for (const auto& subMesh : mesh.SubMeshes)
			{
				if (!RayIntersectsSphere(inverseRay, subMesh.BoundingSphere, intersectionDistance))
					continue;

				const auto& material = IndexOrNull(subMesh.MaterialIndex, obj.Materials);
				auto triangleIntersectionFunc = (material != nullptr && material->BlendFlags.DoubleSided) ? RayIntersectsTriangle : RayIntersectsTriangleSingleSided;

				ForEachSubMeshTriangle(mesh, subMesh, [&](auto& triangle)
				{
					if (!triangleIntersectionFunc(inverseRay, triangle.data(), intersectionDistance) || (intersectionDistance < nearPlane))
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

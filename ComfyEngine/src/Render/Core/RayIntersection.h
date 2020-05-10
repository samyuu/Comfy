#pragma once
#include "Graphics/Camera.h"
#include "Graphics/Auth3D/Transform.h"
#include "Graphics/Auth3D/BoundingTypes.h"
#include "Graphics/Auth3D/ObjSet.h"

#include <glm/gtx/intersect.hpp>

namespace Comfy::Render
{
	inline bool Intersects(const vec3& viewPoint, const vec3& ray, const vec3* trianglePoints, float& outIntersectionDistance)
	{
		vec2 baryPosition;
		return glm::intersectRayTriangle(viewPoint, ray, trianglePoints[0], trianglePoints[1], trianglePoints[2], baryPosition, outIntersectionDistance);
	}

	inline bool Intersects(const vec3& viewPoint, const vec3& ray, const Sphere& sphere, float& outIntesectionDistance)
	{
		return glm::intersectRaySphere(viewPoint, ray, sphere.Center, (sphere.Radius * sphere.Radius), outIntesectionDistance);
	}

	inline bool Intersects(const vec3& viewPoint, const vec3& ray, const Obj& obj, const Transform& transform, float& outIntersectionDistance)
	{
		float intersectionDistance = 0.0f;
		if (!Intersects(viewPoint, ray, obj.BoundingSphere * transform, intersectionDistance))
			return false;

		const mat4 transformMatrix = transform.CalculateMatrix();

		float closestDistance = intersectionDistance;
		const SubMesh* closestSubMesh = nullptr;

		for (const auto& mesh : obj.Meshes)
		{
			if (!Intersects(viewPoint, ray, mesh.BoundingSphere * transform, intersectionDistance))
				continue;

			for (const auto& subMesh : mesh.SubMeshes)
			{
				if (!Intersects(viewPoint, ray, subMesh.BoundingSphere * transform, intersectionDistance))
					continue;

				const PrimitiveType primitive = subMesh.Primitive;
				if (primitive == PrimitiveType::TriangleStrip || primitive == PrimitiveType::Triangles || primitive == PrimitiveType::TriangleFan)
				{
					if (subMesh.GetIndexFormat() != IndexFormat::U16)
						continue;

					const auto& indices = *subMesh.GetIndicesU16();

					const size_t triangleIndexStep = (primitive == PrimitiveType::Triangles) ? 3 : 1;
					for (size_t i = 0; i < indices.size() - 2; i += triangleIndexStep)
					{
						// TODO: Store last two positions so to avoid transforming them multiple times
						const std::array<vec3, 3> triangle =
						{
							transformMatrix * vec4(mesh.VertexData.Positions[indices[i + 0]], 1.0f),
							transformMatrix * vec4(mesh.VertexData.Positions[indices[i + 1]], 1.0f),
							transformMatrix * vec4(mesh.VertexData.Positions[indices[i + 2]], 1.0f),
						};

						if (!Intersects(viewPoint, ray, triangle.data(), intersectionDistance))
							continue;

						if (intersectionDistance < closestDistance || closestSubMesh == nullptr)
						{
							closestDistance = intersectionDistance;
							closestSubMesh = &subMesh;
						}
					}
				}
				else if (intersectionDistance < closestDistance || closestSubMesh == nullptr)
				{
					closestDistance = intersectionDistance;
					closestSubMesh = &subMesh;
				}
			}
		}

		outIntersectionDistance = closestDistance;
		return (closestSubMesh != nullptr);
	}
}

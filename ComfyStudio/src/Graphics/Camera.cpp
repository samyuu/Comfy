#define GLM_FORCE_SWIZZLE
#include "Camera.h"
#undef GLM_FORCE_SWIZZLE

namespace Graphics
{
	namespace
	{
		static constexpr mat4 IdentityMatrix = glm::identity<mat4>();
	}

	void PerspectiveCamera::UpdateMatrices()
	{
		view = glm::lookAt(ViewPoint, Interest, UpDirection);
		projection = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);

		viewProjection = projection * view;

		const mat4 viewProjectionRows = glm::transpose(viewProjection);
		frustum.Planes[0] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[0]);
		frustum.Planes[1] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[0]);
		frustum.Planes[2] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[1]);
		frustum.Planes[3] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[1]);
		frustum.Planes[4] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[2]);
		frustum.Planes[5] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[2]);
	}

	const mat4& PerspectiveCamera::GetView() const
	{
		return view;
	}

	const mat4& PerspectiveCamera::GetProjection() const
	{
		return projection;
	}

	const mat4& PerspectiveCamera::GetViewProjection() const
	{
		return viewProjection;
	}

	vec2 PerspectiveCamera::ProjectPointNormalizedScreen(vec3 worldPosition) const
	{
		const vec4 projectedPosition = viewProjection * vec4(worldPosition, 1.0f);

		// NOTE: Near plane culling
		if (projectedPosition.w <= 0.0f)
			return vec2(std::numeric_limits<float>::infinity());

		// NOTE: Perspective division
		const vec3 perspectivePosition = vec3(projectedPosition.xyz) / projectedPosition.w;

		// NOTE: Center around top left origin like all other window coordinates are
		const vec2 normalizedScreenPosition =
		{
			(perspectivePosition.x + 1.0f) / 2.0f,
			(1.0f - perspectivePosition.y) / 2.0f,
		};

		return normalizedScreenPosition;
	}

	bool PerspectiveCamera::IntersectsViewFrustum(const Sphere& worldSpaceSphere) const
	{
		for (const auto& plane : frustum.Planes)
		{
			if (glm::dot(plane, vec4(worldSpaceSphere.Center, 1.0f)) <= -worldSpaceSphere.Radius)
				return false;
		}

		return true;
	}

	void OrthographicCamera::UpdateMatrices()
	{
		constexpr float projectionLeft = 0.0f;
		constexpr float projectionTop = 0.0f;

		view = glm::translate(IdentityMatrix, vec3(-Position.x, -Position.y, 0.0f)) * glm::scale(IdentityMatrix, vec3(Zoom, Zoom, 1.0f));
		projection = glm::ortho(projectionLeft, ProjectionSize.x, ProjectionSize.y, projectionTop, NearPlane, FarPlane);

		viewProjection = projection * view;
	}

	const mat4& OrthographicCamera::GetView() const
	{
		return view;
	}

	const mat4& OrthographicCamera::GetProjection() const
	{
		return projection;
	}

	const mat4& OrthographicCamera::GetViewProjection() const
	{
		return viewProjection;
	}

	vec3 PerspectiveCamera::ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace)
	{
		return (glm::inverse(matrix) * vec4(screenSpace, 1.0f)).xyz;
	}

	vec3 PerspectiveCamera::WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace)
	{
		return (matrix * vec4(worldSpace, 1.0f)).xyz;
	}

	vec2 OrthographicCamera::GetProjectionCenter() const
	{
		return ProjectionSize * 0.5f;
	}

	vec2 OrthographicCamera::ScreenToWorldSpace(const vec2& screenSpace) const
	{
		return OrthographicCamera::ScreenToWorldSpace(view, screenSpace);
	}

	vec2 OrthographicCamera::WorldToScreenSpace(const vec2& worldSpace) const
	{
		return OrthographicCamera::WorldToScreenSpace(view, worldSpace);
	}

	vec2 OrthographicCamera::ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace)
	{
		return (glm::inverse(matrix) * vec4(screenSpace, 0.0f, 1.0f)).xy;
	}

	vec2 OrthographicCamera::WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace)
	{
		return (matrix * vec4(worldSpace, 0.0f, 1.0f)).xy;
	}
}

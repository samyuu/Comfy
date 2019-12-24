#define GLM_FORCE_SWIZZLE
#include "Camera.h"
#undef GLM_FORCE_SWIZZLE

namespace Graphics
{
	namespace
	{
		static const mat4 IdentityMatrix = mat4(1.0f);
	}

	void PerspectiveCamera::UpdateMatrices()
	{
		view = glm::lookAt(Position, Target, UpDirection);
		projection = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);

		viewProjection = projection * view;
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

#define GLM_FORCE_SWIZZLE
#include "Camera.h"
#undef GLM_FORCE_SWIZZLE

vec3 ICamera::ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace)
{
	return (glm::inverse(matrix) * vec4(screenSpace, 1.0f)).xyz;
}

vec3 ICamera::WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace)
{
	return (matrix * vec4(worldSpace, 1.0f)).xyz;
}

vec2 ICamera::ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace)
{
	return (glm::inverse(matrix) * vec4(screenSpace, 0.0f, 1.0f)).xy;
}

vec2 ICamera::WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace)
{
	return (matrix * vec4(worldSpace, 0.0f, 1.0f)).xy;
}

const mat4 ICamera::identityMatrix = mat4(1.0f);

void PerspectiveCamera::UpdateMatrices()
{
	viewMatrix = glm::lookAt(Position, Target, UpDirection);
	projectionMatrix = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);
}

const mat4& PerspectiveCamera::GetViewMatrix() const
{
	return viewMatrix;
}

const mat4& PerspectiveCamera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

void OrthographicCamera::UpdateMatrices()
{
	constexpr float projectionLeft = 0.0f;
	constexpr float projectionTop = 0.0f;

	viewMatrix = glm::translate(ICamera::identityMatrix, vec3(-Position.x, -Position.y, 0.0f)) * glm::scale(ICamera::identityMatrix, vec3(Zoom, Zoom, 1.0f));
	projectionMatrix = glm::ortho(projectionLeft, ProjectionSize.x, ProjectionSize.y, projectionTop, NearPlane, FarPlane);
}

const mat4& OrthographicCamera::GetViewMatrix() const
{
	return viewMatrix;
}

const mat4& OrthographicCamera::GetProjectionMatrix() const
{
	return projectionMatrix;
}

vec2 OrthographicCamera::GetProjectionCenter() const
{
	return vec2(ProjectionSize.x, ProjectionSize.y) * 0.5f;
}

vec2 OrthographicCamera::ScreenToWorldSpace(const vec2& screenSpace) const
{
	return ICamera::ScreenToWorldSpace(viewMatrix, screenSpace);
}

vec2 OrthographicCamera::WorldToScreenSpace(const vec2& worldSpace) const
{
	return ICamera::WorldToScreenSpace(viewMatrix, worldSpace);
}

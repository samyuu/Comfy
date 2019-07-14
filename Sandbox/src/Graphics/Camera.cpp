#define GLM_FORCE_SWIZZLE
#include "Camera.h"
#undef GLM_FORCE_SWIZZLE

vec3 ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace)
{
	return (glm::inverse(matrix) * vec4(screenSpace, 1.0f)).xyz;
}

vec3 WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace)
{
	return (matrix * vec4(worldSpace, 1.0f)).xyz;
}

vec2 ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace)
{
	return (glm::inverse(matrix) * vec4(screenSpace, 0.0f, 1.0f)).xy;
}

vec2 WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace)
{
	return (matrix * vec4(worldSpace, 0.0f, 1.0f)).xy;
}

void PerspectiveCamera::Update()
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

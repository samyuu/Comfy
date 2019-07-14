#pragma once
#include "Types.h"

constexpr vec3 UP_DIRECTION = vec3(0.0f, 1.0f, 0.0f);

vec3 ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace);
vec3 WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace);

vec2 ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace);
vec2 WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace);

class PerspectiveCamera
{
public:
	vec3 Position = vec3(0, 0, 3);
	vec3 Target;

	vec3 UpDirection = UP_DIRECTION;
	//float Rotation = 0.0f;

	float FieldOfView = 90.0f;
	float AspectRatio = 16.0f / 9.0f;
	float NearPlane = 0.001f;
	float FarPlane = 3939.0f;

	void Update();

	const mat4& GetViewMatrix() const;
	const mat4& GetProjectionMatrix() const;

protected:
	mat4 viewMatrix;
	mat4 projectionMatrix;
};
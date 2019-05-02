#pragma once
#include "../pch.h"

constexpr vec3 UP_DIRECTION = vec3(0.0f, 1.0f, 0.0f);

class Camera
{
public:
	vec3 Position = vec3(0, 0, 3);
	vec3 Target;

	// vec3 Direction;
	// vec3 Front = vec3(0, 0, -1);

	// vec3 Right;
	// vec3 Up;
	vec3 UpDirection = UP_DIRECTION;

	//float Rotation = 0.0f;

	float FieldOfView = 90.0f;
	float AspectRatio = 16.0f / 9.0f;
	float NearPlane = 0.001f;
	float FarPlane = 3939.0f;

	void Update();

	inline mat4& GetViewMatrix() { return viewMatrix; };
	inline mat4& GetProjectionMatrix() { return projectionMatrix; };

protected:
	mat4 viewMatrix;
	mat4 projectionMatrix;
};
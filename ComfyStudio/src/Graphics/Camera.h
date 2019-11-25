#pragma once
#include "Types.h"

namespace Graphics
{
	constexpr vec3 Vec3_UpDirection = vec3(0.0f, 1.0f, 0.0f);

	class ICamera
	{
	public:
		virtual void UpdateMatrices() = 0;

		virtual const mat4& GetViewMatrix() const = 0;
		virtual const mat4& GetProjectionMatrix() const = 0;

	public:
		static vec3 ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace);
		static vec3 WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace);

		static vec2 ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace);
		static vec2 WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace);

	protected:
		static const mat4& GetIdentityMatrix();
	};

	class PerspectiveCamera final : public ICamera
	{
	public:
		vec3 Position = vec3(3.45f, 1.0f, 0.0f);
		vec3 Target;

		vec3 UpDirection = Vec3_UpDirection;

		float FieldOfView = 90.0f;
		float AspectRatio = 16.0f / 9.0f;
		float NearPlane = 0.1f;
		float FarPlane = 6000.0f;

	public:
		void UpdateMatrices() override;

		const mat4& GetViewMatrix() const override;
		const mat4& GetProjectionMatrix() const override;

	private:
		mat4 viewMatrix;
		mat4 projectionMatrix;
	};

	class OrthographicCamera final : public ICamera
	{
	public:
		float Zoom = 1.0f;
		vec2 Position = vec2(0.0f, 0.0f);
		vec2 ProjectionSize = vec2(-1.0f);

		const float NearPlane = 0.0f;
		const float FarPlane = 1.0f;

	public:
		void UpdateMatrices() override;

		const mat4& GetViewMatrix() const override;
		const mat4& GetProjectionMatrix() const override;

		vec2 GetProjectionCenter() const;

		vec2 ScreenToWorldSpace(const vec2& screenSpace) const;
		vec2 WorldToScreenSpace(const vec2& worldSpace) const;

	private:
		mat4 viewMatrix;
		mat4 projectionMatrix;
	};
}

#pragma once
#include "Types.h"

namespace Graphics
{
	class ICamera
	{
	public:
		static constexpr vec3 UpDirection = vec3(0.0f, 1.0f, 0.0f);

	public:
		virtual void UpdateMatrices() = 0;

		virtual const mat4& GetView() const = 0;
		virtual const mat4& GetProjection() const = 0;
		virtual const mat4& GetViewProjection() const = 0;
	};

	class PerspectiveCamera final : public ICamera
	{
	public:
		vec3 Position = vec3(3.45f, 1.0f, 0.0f);
		vec3 Target = vec3(0.0f, 0.0f, 0.0f);

		float FieldOfView = 90.0f;
		float AspectRatio = 16.0f / 9.0f;

		float NearPlane = 0.1f;
		float FarPlane = 6000.0f;

	public:
		void UpdateMatrices() override;

		const mat4& GetView() const override;
		const mat4& GetProjection() const override;
		const mat4& GetViewProjection() const override;

	public:
		static vec3 ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace);
		static vec3 WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace);

	private:
		mat4 view, projection, viewProjection;
	};

	class OrthographicCamera final : public ICamera
	{
	public:
		float Zoom = 1.0f;

		vec2 Position = vec2(0.0f, 0.0f);
		vec2 ProjectionSize = vec2(0.0f, 0.0f);

		const float NearPlane = 0.0f;
		const float FarPlane = 1.0f;

	public:
		void UpdateMatrices() override;

		const mat4& GetView() const override;
		const mat4& GetProjection() const override;
		const mat4& GetViewProjection() const override;

		vec2 GetProjectionCenter() const;

		vec2 ScreenToWorldSpace(const vec2& screenSpace) const;
		vec2 WorldToScreenSpace(const vec2& worldSpace) const;

	public:
		static vec2 ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace);
		static vec2 WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace);

	private:
		mat4 view, projection, viewProjection;
	};
}

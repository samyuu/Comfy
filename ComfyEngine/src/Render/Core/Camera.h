#pragma once
#include "Types.h"
#include "Graphics/Auth3D/BoundingTypes.h"
#include "Graphics/Auth3D/Ray.h"

namespace Comfy::Render
{
	class ICamera
	{
	public:
		static constexpr vec3 UpDirection = vec3(0.0f, 1.0f, 0.0f);
		static constexpr vec3 ForwardDirection = vec3(0.0f, 0.0f, -1.0f);
		static constexpr vec3 RightDirection = vec3(1.0f, 0.0f, 0.0f);

		virtual void UpdateMatrices() = 0;

		virtual const mat4& GetView() const = 0;
		virtual const mat4& GetProjection() const = 0;
		virtual const mat4& GetViewProjection() const = 0;
	};

	class Camera3D final : public ICamera
	{
	public:
		vec3 ViewPoint = vec3(3.45f, 1.0f, 0.0f);
		vec3 Interest = vec3(0.0f, 0.0f, 0.0f);

		float FieldOfView = 90.0f;
		float AspectRatio = 16.0f / 9.0f;

		float NearPlane = 0.050f;
		float FarPlane = 6000.0f;

		// NOTE: Prevent potential disorientation caused by sudden perspective / orthographic switching (Perspective = 0.0f, Orthographic = 1.0f)
		float OrthographicLerp = 0.0f;

	public:
		void UpdateMatrices() override;

		const mat4& GetView() const override;
		const mat4& GetProjection() const override;
		const mat4& GetViewProjection() const override;

	public:
		vec2 ProjectPointNormalizedScreen(vec3 worldPosition) const;
		Graphics::Ray CastRay(vec2 normalizeScreenPosition) const;

		bool IntersectsViewFrustum(const Graphics::Sphere& worldSpaceSphere) const;

	public:
		static vec3 ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace);
		static vec3 WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace);

	private:
		struct Frustum
		{
			std::array<vec4, 6> Planes;
		} frustum;

		mat4 view, projection, viewProjection;
	};

	class Camera2D final : public ICamera
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

	public:
		vec2 GetProjectionCenter() const;
		std::pair<vec2, vec2> GetFullScreenCoveringQuad() const;

		vec2 ScreenToWorldSpace(const vec2& screenSpace) const;
		vec2 WorldToScreenSpace(const vec2& worldSpace) const;

	public:
		void CenterAndZoomToFit(vec2 targetSize);

	public:
		static vec2 ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace);
		static vec2 WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace);

	private:
		mat4 view, projection, viewProjection;
	};
}

#define GLM_FORCE_SWIZZLE
#include "Camera.h"
#undef GLM_FORCE_SWIZZLE

namespace Comfy::Render
{
	namespace
	{
		constexpr float Lerp(float inputA, float inputB, float delta)
		{
			return (inputA * delta + inputB * (1.0f - delta));
		}

		mat4 Lerp(const mat4& inputA, const mat4& inputB, float delta)
		{
			mat4 result;
			for (auto c = 0; c < mat4::length(); c++)
			{
				for (auto r = 0; r < mat4::length(); r++)
					result[c][r] = Lerp(inputA[c][r], inputB[c][r], delta);
			}
			return result;
		}
	}

	void Camera3D::UpdateMatrices()
	{
		view = glm::lookAt(ViewPoint, Interest, UpDirection);
		projection = glm::perspective(glm::radians(FieldOfView), AspectRatio, NearPlane, FarPlane);

		if (OrthographicLerp != 0.0f)
		{
			OrthographicLerp = Clamp(OrthographicLerp, 0.0f, 1.0f);

			const auto orthoSize = glm::normalize(vec2(AspectRatio, 1.0f)) * glm::distance(ViewPoint, Interest);
			const auto orthoProjection = glm::ortho(
				-0.5f * orthoSize.x,
				+0.5f * orthoSize.x,
				-0.5f * orthoSize.y,
				+0.5f * orthoSize.y,
				NearPlane, FarPlane);

			projection = Lerp(orthoProjection, projection, OrthographicLerp);
		}

		viewProjection = (projection * view);

		const mat4 viewProjectionRows = glm::transpose(viewProjection);
		frustum.Planes[0] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[0]);
		frustum.Planes[1] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[0]);
		frustum.Planes[2] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[1]);
		frustum.Planes[3] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[1]);
		frustum.Planes[4] = glm::normalize(viewProjectionRows[3] + viewProjectionRows[2]);
		frustum.Planes[5] = glm::normalize(viewProjectionRows[3] - viewProjectionRows[2]);
	}

	const mat4& Camera3D::GetView() const
	{
		return view;
	}

	const mat4& Camera3D::GetProjection() const
	{
		return projection;
	}

	const mat4& Camera3D::GetViewProjection() const
	{
		return viewProjection;
	}

	vec2 Camera3D::ProjectPointNormalizedScreen(vec3 worldPosition) const
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

	Graphics::Ray Camera3D::CastRay(vec2 normalizeScreenPosition) const
	{
		auto ray = Graphics::Ray {};
		const auto clipSpacePosition = vec4((normalizeScreenPosition.x * 2.0f) - 1.0f, 1.0f - ((normalizeScreenPosition.y * 2.0f)), 1.0f, 1.0f);

		// NOTE: Ray casting while ortho lerping not (== 0.0f || == 1.0f) will not be accurate
		if (OrthographicLerp >= 0.999999f)
		{
			ray.Direction = glm::normalize(Interest - ViewPoint);

			const auto orientation = glm::mat4_cast(glm::inverse(glm::quatLookAt(ray.Direction, Interest)));
			const auto orthoSize = glm::normalize(vec2(AspectRatio, 1.0f)) * glm::distance(ViewPoint, Interest);

			ray.Origin = ViewPoint;
			ray.Origin += (orthoSize.x * clipSpacePosition.x * 0.5f) * glm::normalize(vec3((vec4(RightDirection, 1.0f) * orientation)));
			ray.Origin += (orthoSize.y * clipSpacePosition.y * 0.5f) * glm::normalize(vec3((vec4(UpDirection, 1.0f) * orientation)));
		}
		else
		{
			ray.Origin = ViewPoint;

			const auto inverseViewProjection = glm::inverse(viewProjection);
			ray.Direction = glm::normalize(vec3(inverseViewProjection * clipSpacePosition));
		}

		return ray;
	}

	bool Camera3D::IntersectsViewFrustum(const Graphics::Sphere& worldSpaceSphere) const
	{
		for (const auto& plane : frustum.Planes)
		{
			if (glm::dot(plane, vec4(worldSpaceSphere.Center, 1.0f)) <= -worldSpaceSphere.Radius)
				return false;
		}

		return true;
	}

	void Camera2D::UpdateMatrices()
	{
		constexpr float projectionLeft = 0.0f;
		constexpr float projectionTop = 0.0f;

		view = glm::translate(mat4(1.0f), vec3(-Position.x, -Position.y, 0.0f)) * glm::scale(mat4(1.0f), vec3(Zoom, Zoom, 1.0f));
		projection = glm::ortho(projectionLeft, ProjectionSize.x, ProjectionSize.y, projectionTop, NearPlane, FarPlane);

		viewProjection = projection * view;
	}

	const mat4& Camera2D::GetView() const
	{
		return view;
	}

	const mat4& Camera2D::GetProjection() const
	{
		return projection;
	}

	const mat4& Camera2D::GetViewProjection() const
	{
		return viewProjection;
	}

	vec3 Camera3D::ScreenToWorldSpace(const mat4& matrix, const vec3& screenSpace)
	{
		return (glm::inverse(matrix) * vec4(screenSpace, 1.0f)).xyz;
	}

	vec3 Camera3D::WorldToScreenSpace(const mat4& matrix, const vec3& worldSpace)
	{
		return (matrix * vec4(worldSpace, 1.0f)).xyz;
	}

	vec2 Camera2D::GetProjectionCenter() const
	{
		return ProjectionSize * 0.5f;
	}

	std::pair<vec2, vec2> Camera2D::GetFullScreenCoveringQuad() const
	{
		const vec2 position = (Position / Zoom);
		const vec2 size = (ProjectionSize / Zoom);

		return std::make_pair(position, size);
	}

	vec2 Camera2D::ScreenToWorldSpace(const vec2& screenSpace) const
	{
		return Camera2D::ScreenToWorldSpace(view, screenSpace);
	}

	vec2 Camera2D::WorldToScreenSpace(const vec2& worldSpace) const
	{
		return Camera2D::WorldToScreenSpace(view, worldSpace);
	}

	void Camera2D::CenterAndZoomToFit(vec2 targetSize)
	{
		const float targetAspectRatio = (targetSize.x / targetSize.y);
		const float projectionAspectRatio = (ProjectionSize.x / ProjectionSize.y);

		if (targetAspectRatio < projectionAspectRatio)
		{
			Zoom = (ProjectionSize.y / targetSize.y);
			Position = vec2((targetSize.x * Zoom - ProjectionSize.x) / 2.0f, 0.0f);
		}
		else
		{
			Zoom = (ProjectionSize.x / targetSize.x);
			Position = vec2(0.0f, (targetSize.y * Zoom - ProjectionSize.y) / 2.0f);
		}
	}

	vec2 Camera2D::ScreenToWorldSpace(const mat4& matrix, const vec2& screenSpace)
	{
		return (glm::inverse(matrix) * vec4(screenSpace, 0.0f, 1.0f)).xy;
	}

	vec2 Camera2D::WorldToScreenSpace(const mat4& matrix, const vec2& worldSpace)
	{
		return (matrix * vec4(worldSpace, 0.0f, 1.0f)).xy;
	}
}

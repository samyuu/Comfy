#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy
{
	namespace CubicBezier
	{
		constexpr vec2 GetPoint(vec2 start, vec2 end, vec2 startControl, vec2 endControl, f32 t)
		{
			const f32 oneMinusT = 1.0f - t;
			const f32 oneMinusTPow2 = oneMinusT * oneMinusT;
			const f32 tPow2 = t * t;

			return
				start * (oneMinusTPow2 * oneMinusT) +
				startControl * (3.0f * oneMinusTPow2 * t) +
				endControl * (3.0f * oneMinusT * tPow2) +
				end * (tPow2 * t);
		}

		inline std::array<vec2, 3> GetPointTangentNormal(vec2 start, vec2 end, vec2 startControl, vec2 endControl, f32 t)
		{
			const f32 oneMinusT = 1.0f - t;
			const f32 oneMinusTPow2 = oneMinusT * oneMinusT;
			const f32 tPow2 = t * t;

			const auto point =
				start * (oneMinusTPow2 * oneMinusT) +
				startControl * (3.0f * oneMinusTPow2 * t) +
				endControl * (3.0f * oneMinusT * tPow2) +
				end * (tPow2 * t);

			const auto tangent = glm::normalize(
				start * (-oneMinusTPow2) +
				startControl * (3.0f * oneMinusTPow2 - 2.0f * oneMinusT) +
				endControl * (-3.0f * tPow2 + 2.0f * t) +
				end * (tPow2));

			const auto normal = vec2(tangent.y, -tangent.x);
			return { point, tangent, normal };
		}
	}
}

#include "CameraAxisIndication.h"

namespace Editor
{
	using namespace Graphics;

	namespace
	{
		std::array<vec2, 4> GetAxisScreenPositions(const PerspectiveCamera& camera)
		{
			const vec3 viewDirection = glm::normalize(camera.ViewPoint - camera.Interest);
			const mat4 viewProjection = glm::ortho(+1.0f, -1.0f, +1.0f, -1.0f) * glm::lookAt(vec3(0.0f), viewDirection, camera.UpDirection);

			constexpr float scale = 1.0f, origin = 0.0f;
			constexpr std::array<vec3, 4> axisWorldPositions =
			{
				vec3(+scale, origin, origin),
				vec3(origin, +scale, origin),
				vec3(origin, origin, -scale),
				vec3(origin, origin, origin),
			};

			std::array<vec2, 4> screenPositions;
			for (size_t i = 0; i < screenPositions.size(); i++)
				screenPositions[i] = vec2(viewProjection * vec4(axisWorldPositions[i], 1.0f)) + vec2(1.0f);
			return screenPositions;
		}
	}

	void DrawCameraAxisIndicationGui(ImDrawList* drawList, const PerspectiveCamera& camera, vec2 indicatorCenter, float indicatorSize, float indicatorPadding, vec2 textOffset)
	{
		std::array<vec2, 4> screenPositions = GetAxisScreenPositions(camera);
		for (size_t i = 0; i < screenPositions.size(); i++)
			screenPositions[i] = indicatorCenter - indicatorSize + (screenPositions[i]) * indicatorSize;

		constexpr std::array<vec4, 4> axisColors =
		{
			vec4(1.000f, 0.167f, 0.020f, 1.000f),
			vec4(0.000f, 0.904f, 0.213f, 1.000f),
			vec4(0.000f, 0.345f, 0.841f, 1.000f),
			vec4(0.150f, 0.150f, 0.150f, 0.250f),
		};

		std::array<int, 3> axisDrawOrder = { 0, 2, 1 };
		std::array<char, 3> axisNames = { 'X', 'Y', 'Z' };

		const vec3 viewDirection = glm::normalize(camera.ViewPoint - camera.Interest);

		if (glm::atan(viewDirection.y, viewDirection.x) < glm::half_pi<float>())
			std::swap(axisDrawOrder[0], axisDrawOrder[1]);

		if (viewDirection.y < 0.0f)
			std::swap(axisDrawOrder[1], axisDrawOrder[2]);

		drawList->AddCircleFilled(screenPositions.back(), indicatorSize * 2.0f, ImColor(axisColors.back()), 24);

		for (int i : axisDrawOrder)
			drawList->AddText(nullptr, indicatorSize, screenPositions[i] - textOffset, ImColor(axisColors[i]), &axisNames[i], &axisNames[i] + 1);

		for (int i : axisDrawOrder)
			drawList->AddLine(screenPositions.back(), screenPositions[i], ImColor(axisColors[i]));
	}
}

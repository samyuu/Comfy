#include "CameraAxisIndication.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		std::array<vec2, 4> GetAxisScreenPositions(const Render::Camera3D& camera)
		{
			const vec3 viewDirection = glm::normalize(camera.ViewPoint - camera.Interest);
			const mat4 viewProjection = glm::ortho(+1.0f, -1.0f, +1.0f, -1.0f) * glm::lookAt(vec3(0.0f), viewDirection, camera.UpDirection);

			constexpr std::array<vec3, 4> axisWorldPositions =
			{
				// NOTE: Right-handed Y-up
				vec3(1.0f, 0.0f, 0.0f),
				vec3(0.0f, 1.0f, 0.0f),
				vec3(0.0f, 0.0f, 1.0f),
				vec3(0.0f, 0.0f, 0.0f),
			};

			std::array<vec2, 4> screenPositions;
			for (size_t i = 0; i < screenPositions.size(); i++)
				screenPositions[i] = vec2(viewProjection * vec4(axisWorldPositions[i], 1.0f)) + vec2(1.0f);
			return screenPositions;
		}
	}

	void DrawCameraAxisIndicationGui(ImDrawList* drawList, const Render::Camera3D& camera, vec2 indicatorCenter, float indicatorSize, float indicatorPadding, vec2 textOffset)
	{
		std::array<vec2, 4> screenPositions = GetAxisScreenPositions(camera);
		for (size_t i = 0; i < screenPositions.size(); i++)
			screenPositions[i] = indicatorCenter - indicatorSize + (screenPositions[i]) * indicatorSize;

		constexpr auto colorRed = vec4(0.88f, 0.29f, 0.26f, 1.0f);
		constexpr auto colorGreen = vec4(0.42f, 0.68f, 0.25f, 1.0f);
		constexpr auto colorBlue = vec4(0.26f, 0.62f, 1.00f, 1.0f);
		constexpr auto colorBackground = vec4(0.25f, 0.25f, 0.25f, 0.45f);

		static constexpr std::array<vec4, 3> axisColors = { colorRed, colorGreen, colorBlue, };

		std::array<int, 3> axisDrawOrder = { 0, 1, 2 };
		std::array<char, 3> axisNames = { 'X', 'Y', 'Z' };

		const vec3 viewDirection = glm::normalize(camera.ViewPoint - camera.Interest);

		if (glm::atan(viewDirection.y, viewDirection.x) < glm::half_pi<float>())
			std::swap(axisDrawOrder[0], axisDrawOrder[1]);

		if (viewDirection.y < 0.0f)
			std::swap(axisDrawOrder[1], axisDrawOrder[2]);

		drawList->AddCircleFilled(screenPositions.back(), indicatorSize * 2.0f, ImColor(colorBackground), 24);

		for (int i : axisDrawOrder)
			drawList->AddText(nullptr, indicatorSize, screenPositions[i] - textOffset, ImColor(axisColors[i]), &axisNames[i], &axisNames[i] + 1);

		for (int i : axisDrawOrder)
			drawList->AddLine(screenPositions.back(), screenPositions[i], ImColor(axisColors[i]), 1.0f);
	}
}

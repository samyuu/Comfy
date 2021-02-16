#include "TargetGuiDrawUtil.h"
#include "TargetRenderWindow.h"

namespace Comfy::Studio::Editor
{
	namespace
	{
		constexpr struct ArrowSettingsData
		{
			f32 HeadSpacing = 96.0f;
			f32 HeadEndSpacingFactor = 0.6f;
			f32 HeadSize = 168.0f;
			f32 HeadAngle = 16.0f;
			vec4 BackgroundColor = vec4(0.16f, 0.16f, 0.16f, 0.95f);
		} ArrowSettings;
	}

	void DrawStraightButtonAngleLine(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness)
	{
		const auto angleRadians = glm::radians(properties.Angle) - glm::radians(90.0f);
		const auto angleDirection = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto start = renderWindow.TargetAreaToScreenSpace(properties.Position);
		const auto end = renderWindow.TargetAreaToScreenSpace(properties.Position + (angleDirection * properties.Distance));

		drawList.AddLine(start, end, color, thickness);
	}

	void DrawCurvedButtonPathLine(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness)
	{
		if (properties.Frequency == 0.0f || properties.Amplitude == 0.0f)
		{
			DrawStraightButtonAngleLine(renderWindow, drawList, properties, color, thickness);
		}
		else
		{
			drawList.PathLineTo(renderWindow.TargetAreaToScreenSpace(GetButtonPathSinePoint(0.0f, properties)));
			for (f32 i = CurvedButtonPathStepDistance; i <= 1.0f; i += CurvedButtonPathStepDistance)
				drawList.PathLineTo(renderWindow.TargetAreaToScreenSpace(GetButtonPathSinePoint(i, properties)));
			drawList.PathStroke(color, false, thickness);
		}
	}

	void DrawCurvedButtonPathLineArrowHeads(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness)
	{
		const auto angleRadians = glm::radians(properties.Angle) - glm::radians(90.0f);
		const auto angleDirection = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto start = renderWindow.TargetAreaToScreenSpace(properties.Position);
		const auto end = renderWindow.TargetAreaToScreenSpace(properties.Position + (angleDirection * properties.Distance));

		const auto headSpacing = ArrowSettings.HeadSpacing * renderWindow.GetCamera().Zoom;
		const auto headSize = (ArrowSettings.HeadSize * renderWindow.GetCamera().Zoom) / 2.0f;
		const auto headRadians = glm::radians(ArrowSettings.HeadAngle);

		const auto headRadiansL = (angleRadians - headRadians);
		const auto headRadiansR = (angleRadians + headRadians);
		const auto headDirectionL = vec2(glm::cos(headRadiansL), glm::sin(headRadiansL));
		const auto headDirectionR = vec2(glm::cos(headRadiansR), glm::sin(headRadiansR));
		const auto backgroundColor = Gui::ColorConvertFloat4ToU32(ArrowSettings.BackgroundColor * Gui::ColorConvertU32ToFloat4(color));

		const auto startEndDistance = (properties.Distance <= 0.0f) ? 0.0f : glm::distance(start, end);
		const auto headCount = std::max(1, static_cast<i32>(glm::floor(startEndDistance / headSpacing)));

		for (i32 head = 0; head < headCount; head++)
		{
			const auto headDistance = ((head + 0) * headSpacing) + (headSpacing * ArrowSettings.HeadEndSpacingFactor) + (thickness / 2.0f);
			const auto headDistanceNext = ((head + 1) == headCount) ? startEndDistance : glm::min(startEndDistance, ((head + 1) * headSpacing));

			const auto headStart = start + (headDistance * angleDirection);
			const auto headEnd = start + (headDistanceNext * angleDirection);

			drawList.AddLine(headStart, headEnd, color, thickness);
		}

		for (i32 head = 0; head < headCount; head++)
		{
			const auto headDistance = (head * headSpacing);

			const auto headStart = start + (headDistance * angleDirection);
			const auto headEnd = start + ((headDistance + (headSpacing * ArrowSettings.HeadEndSpacingFactor)) * angleDirection);

			const auto headEndL = headStart + (headDirectionL * headSize);
			const auto headEndR = headStart + (headDirectionR * headSize);

			// NOTE: Primarily to hide the AA triangle seams at some angles
			drawList.AddLine(headStart, headEnd, backgroundColor, 1.0f);
			drawList.AddTriangleFilled(headStart, headEndL, headEnd, backgroundColor);
			drawList.AddTriangleFilled(headEnd, headEndR, headStart, backgroundColor);

			drawList.AddLine(headStart, headEndL, color, thickness);
			drawList.AddLine(headStart, headEndR, color, thickness);
			drawList.AddLine(headEndL, headEnd, color, thickness);
			drawList.AddLine(headEndR, headEnd, color, thickness);
		}
	}

	void DrawButtonPathArrowHead(TargetRenderWindow& renderWindow, ImDrawList& drawList, vec2 targetSpacePos, vec2 direction, u32 color, f32 thickness)
	{
		const auto angleRadians = glm::atan(direction.y, direction.x);
		const auto angleDirection = vec2(glm::cos(angleRadians), glm::sin(angleRadians));

		const auto headSpacing = ArrowSettings.HeadSpacing * renderWindow.GetCamera().Zoom;
		const auto headSize = (ArrowSettings.HeadSize * renderWindow.GetCamera().Zoom) / 2.0f;
		const auto headRadians = glm::radians(ArrowSettings.HeadAngle);

		const auto headRadiansL = (angleRadians - headRadians);
		const auto headRadiansR = (angleRadians + headRadians);
		const auto headDirectionL = vec2(glm::cos(headRadiansL), glm::sin(headRadiansL));
		const auto headDirectionR = vec2(glm::cos(headRadiansR), glm::sin(headRadiansR));
		const auto backgroundColor = Gui::ColorConvertFloat4ToU32(ArrowSettings.BackgroundColor * Gui::ColorConvertU32ToFloat4(color));

		const auto headStart = renderWindow.TargetAreaToScreenSpace(targetSpacePos);
		const auto headEnd = headStart + ((headSpacing * ArrowSettings.HeadEndSpacingFactor) * angleDirection);

		const auto headEndL = headStart + (headDirectionL * headSize);
		const auto headEndR = headStart + (headDirectionR * headSize);

		drawList.AddLine(headStart, headEnd, backgroundColor, 1.0f);
		drawList.AddTriangleFilled(headStart, headEndL, headEnd, backgroundColor);
		drawList.AddTriangleFilled(headEnd, headEndR, headStart, backgroundColor);

		drawList.AddLine(headStart, headEndL, color, thickness);
		drawList.AddLine(headStart, headEndR, color, thickness);
		drawList.AddLine(headEndL, headEnd, color, thickness);
		drawList.AddLine(headEndR, headEnd, color, thickness);
	}

	void DrawButtonPathArrowHeadCentered(TargetRenderWindow& renderWindow, ImDrawList& drawList, vec2 targetSpacePos, vec2 direction, u32 color, f32 thickness)
	{
		DrawButtonPathArrowHead(renderWindow, drawList, targetSpacePos - (vec2(ArrowSettings.HeadSize * 0.25f) * direction), direction, color, thickness);
	}
}

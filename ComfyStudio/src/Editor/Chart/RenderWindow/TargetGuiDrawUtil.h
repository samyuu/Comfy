#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Render/Render.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class TargetRenderWindow;

	constexpr std::array<u32, EnumCount<ButtonType>()> ButtonTypeColors =
	{
		0xFFCCFE62,
		0xFFD542FF,
		0xFFFEFF62,
		0xFF6412FE,
		0xFF2BD7FF,
		0xFF2BD7FF,
	};

	constexpr u32 GetButtonTypeColorU32(ButtonType type) { return ButtonTypeColors[static_cast<u8>(type)]; }
	constexpr u32 GetButtonTypeColorU32(ButtonType type, u8 alpha) { return (ButtonTypeColors[static_cast<u8>(type)] & 0x00FFFFFF) | (static_cast<u32>(alpha) << 24); }

	constexpr f32 CurvedButtonPathStepDistance = (1.0f / 32.0f);

	void DrawStraightButtonAngleLine(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness);
	void DrawCurvedButtonPathLine(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness);
	void DrawCurvedButtonPathLineArrowHeads(TargetRenderWindow& renderWindow, ImDrawList& drawList, const TargetProperties& properties, u32 color, f32 thickness);
	void DrawButtonPathArrowHead(TargetRenderWindow& renderWindow, ImDrawList& drawList, vec2 targetSpacePos, vec2 direction, u32 color, f32 thickness);
	void DrawButtonPathArrowHeadCentered(TargetRenderWindow& renderWindow, ImDrawList& drawList, vec2 targetSpacePos, vec2 direction, u32 color, f32 thickness);
}

#include "AetTool.h"

namespace Comfy::Studio::Editor
{
	void AetTool::SetSpaceConversionFunctions(const std::function<vec2(vec2)>& worldToScreenSpace, const std::function<vec2(vec2)>& screenToWorldSpace)
	{
		worldToScreenSpaceFunction = worldToScreenSpace;
		screenToWorldSpaceFunction = screenToWorldSpace;
	}

	vec2 AetTool::ToScreenSpace(vec2 worldSpace) const
	{
		return worldToScreenSpaceFunction(worldSpace);
	}

	vec2 AetTool::ToWorldSpace(vec2 screenSpace) const
	{
		return screenToWorldSpaceFunction(screenSpace);
	}
}

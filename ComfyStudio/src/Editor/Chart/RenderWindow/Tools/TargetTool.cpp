#include "TargetTool.h"
#include "TargetPositionTool.h"
#include "TargetRotationTool.h"

namespace Comfy::Studio::Editor
{
	TargetTool::TargetTool(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager)
		: renderWindow(renderWindow), undoManager(undoManager)
	{
	}

	std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> TargetTool::CreateAllToolTypes(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager)
	{
		std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> allTools;
		allTools[static_cast<u8>(TargetToolType::Position)] = std::make_unique<TargetPositionTool>(renderWindow, undoManager);
		allTools[static_cast<u8>(TargetToolType::Rotation)] = std::make_unique<TargetRotationTool>(renderWindow, undoManager);
		return allTools;
	}
}

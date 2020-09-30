#include "TargetTool.h"
#include "TargetPositionTool.h"

namespace Comfy::Studio::Editor
{
	TargetTool::TargetTool(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager)
		: renderWindow(renderWindow), undoManager(undoManager)
	{
	}

	std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> TargetTool::CreateAllToolTypes(TargetRenderWindow& renderWindow, Undo::UndoManager& undoManager)
	{
		std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> allTools;
		allTools[static_cast<u8>(TargetToolType::PositionTool)] = std::make_unique<TargetPositionTool>(renderWindow, undoManager);
		return allTools;
	}
}

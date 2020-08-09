#pragma once
#include "Types.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	// TODO: - Set AetItemType::Surface
	//		 - Should work both horizontally and vertically (?)
	//		 - Drag and drop into AetRenderWindow to place layer
	//		 - Keep aspect ratio for preview "boxes" (write helper function to also be used by RenderWindowBase)
	//		 - 

	class AetContentView
	{
	public:
		// AetContentView(Undo::UndoManager& undoManager, AetItemTypePtr* selectedAetItem);
		AetContentView(const AetTreeView&) = delete;
		// ~AetContentView();

	public:
	};
}

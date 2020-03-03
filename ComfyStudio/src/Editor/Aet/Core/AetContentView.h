#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"

namespace Editor
{
	// TODO: - Set AetItemType::Surface
	//		 - Should work both horizontally and vertically (?)
	//		 - Drag and drop into AetRenderWindow to place layer
	//		 - Keep aspect ratio for preview "boxes" (write helper function to also be used by RenderWindowBase)
	//		 - 

	class AetContentView : public IMutatingEditorComponent
	{
	public:
		//AetContentView(AetCommandManager* commandManager, AetItemTypePtr* selectedAetItem);
		AetContentView(const AetTreeView&) = delete;
		//~AetContentView();

	public:
	};
}

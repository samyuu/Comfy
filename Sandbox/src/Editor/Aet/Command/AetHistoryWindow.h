#pragma once
#include "Editor/Aet/IMutableAetEditorComponent.h"

namespace Editor
{
	class AetHistoryWindow : public IMutableAetEditorComponent
	{
	public:
		AetHistoryWindow(AetCommandManager* commandManager);
		
		bool DrawGui();

	private:
	};
}
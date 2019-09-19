#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"

namespace Editor
{
	class AetHistoryWindow : public IMutatingEditorComponent
	{
	public:
		AetHistoryWindow(AetCommandManager* commandManager);
		
		bool DrawGui();

	private:
	};
}
#pragma once
#include "Editor/Aet/IMutatingEditorComponent.h"

namespace Comfy::Studio::Editor
{
	class AetHistoryWindow : public IMutatingEditorComponent
	{
	public:
		AetHistoryWindow(AetCommandManager* commandManager);
		
		bool DrawGui();

	private:
	};
}

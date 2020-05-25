#pragma once
#include "Types.h"
#include "Editor/Aet/MutatingEditorComponent.h"

namespace Comfy::Studio::Editor
{
	class AetHistoryWindow : public MutatingEditorComponent
	{
	public:
		AetHistoryWindow(AetCommandManager& commandManager);
		~AetHistoryWindow() = default;

	public:
		bool Gui();

	private:
	};
}

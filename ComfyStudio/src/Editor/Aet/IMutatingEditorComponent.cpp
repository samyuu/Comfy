#include "IMutatingEditorComponent.h"

namespace Comfy::Studio::Editor
{
	IMutatingEditorComponent::IMutatingEditorComponent(AetCommandManager* commandManager) : commandManager(commandManager)
	{
	}

	AetCommandManager* IMutatingEditorComponent::GetCommandManager()
	{
		return commandManager;
	}
}

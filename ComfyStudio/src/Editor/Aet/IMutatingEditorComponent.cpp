#include "IMutatingEditorComponent.h"

namespace Comfy::Editor
{
	IMutatingEditorComponent::IMutatingEditorComponent(AetCommandManager* commandManager) : commandManager(commandManager)
	{
	}

	AetCommandManager* IMutatingEditorComponent::GetCommandManager()
	{
		return commandManager;
	}
}

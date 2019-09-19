#include "IMutatingEditorComponent.h"

namespace Editor
{
	IMutatingEditorComponent::IMutatingEditorComponent(AetCommandManager* commandManager) : commandManager(commandManager)
	{
	}

	AetCommandManager* IMutatingEditorComponent::GetCommandManager()
	{
		return commandManager;
	}
}
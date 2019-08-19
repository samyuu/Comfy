#include "IMutableAetEditorComponent.h"

namespace Editor
{
	IMutableAetEditorComponent::IMutableAetEditorComponent(AetCommandManager* commandManager) : commandManager(commandManager)
	{
	}

	AetCommandManager* IMutableAetEditorComponent::GetCommandManager()
	{
		return commandManager;
	}
}
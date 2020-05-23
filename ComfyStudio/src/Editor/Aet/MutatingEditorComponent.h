#pragma once
#include "Command/AetCommandManager.h"

namespace Comfy::Studio::Editor
{
	class MutatingEditorComponent : NonCopyable
	{
	public:
		MutatingEditorComponent(AetCommandManager& commandManager) : commandManager(commandManager) {}
		virtual ~MutatingEditorComponent() = default;

	protected:
		AetCommandManager& commandManager;
	};
}

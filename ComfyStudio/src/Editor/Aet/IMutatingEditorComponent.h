#pragma once
#include "Command/AetCommandManager.h"

namespace Comfy::Studio::Editor
{
	class IMutatingEditorComponent : NonCopyable
	{
	public:
		IMutatingEditorComponent(AetCommandManager* commandManager);

	protected:
		AetCommandManager* GetCommandManager();
		
	private:
		AetCommandManager* commandManager;
	};
}

#pragma once
#include "Command/AetCommandManager.h"

namespace Editor
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
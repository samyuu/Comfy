#pragma once
#include "Command/AetCommandManager.h"

namespace Editor
{
	class IMutableAetEditorComponent
	{
	public:
		IMutableAetEditorComponent(AetCommandManager* commandManager);

	protected:
		AetCommandManager* GetCommandManager();
		
	private:
		AetCommandManager* commandManager;
	};
}
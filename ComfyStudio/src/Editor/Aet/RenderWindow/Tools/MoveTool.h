#pragma once
#include "AetTool.h"

namespace Comfy::Studio::Editor
{
	class MoveTool : public AetTool
	{
	public:
		const char* GetIcon() const override;
		const char* GetName() const override;
		AetToolType GetType() const override;
		Input::KeyCode GetShortcutKey() const override;

		void DrawContextMenu() override;

	private:
	};
}

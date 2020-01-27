#pragma once
#include "AetTool.h"

namespace Editor
{
	class RotateTool : public AetTool
	{
	public:
		const char* GetIcon() const override;
		const char* GetName() const override;
		AetToolType GetType() const override;
		KeyCode GetShortcutKey() const override;

		void DrawContextMenu() override;

	private:
	};
}
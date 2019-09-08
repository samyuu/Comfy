#pragma once
#include "AetTool.h"

namespace Editor
{
	class RotationTool : public AetTool
	{
	public:
		virtual const char* GetIcon() const override;
		virtual const char* GetName() const override;
		virtual AetToolType GetType() const override;
		virtual KeyCode GetShortcutKey() const override;

		virtual void DrawContextMenu() override;

	private:
	};
}
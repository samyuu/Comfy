#pragma once
#include "Types.h"
#include "CoreTypes.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor;

	class ChartEditorManual : NonCopyable
	{
	public:
		ChartEditorManual(ChartEditor& parent);
		~ChartEditorManual() = default;

	public:
		void Gui();

	private:
		ChartEditor& chartEditor;

	private:
	};
}

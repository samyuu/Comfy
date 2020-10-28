#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	enum class DynamicSyncPreset : u8
	{
		VerticalLeft,
		VerticalRight,
		HorizontalUp,
		HorizontalDown,
		Square,
		Triangle,
		Count
	};

	class PresetWindow : NonCopyable
	{
	public:
		PresetWindow(Undo::UndoManager& undoManager);
		~PresetWindow() = default;

	public:
		void Gui(Chart& chart);
		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);

	private:
		Undo::UndoManager& undoManager;

		const Graphics::SprSet* editorSprites;
		struct SpriteCache
		{
			std::array<const Graphics::Spr*, EnumCount<DynamicSyncPreset>()> DynamicSyncPresetIcons;
		} sprites = {};
	};
}

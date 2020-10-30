#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "TargetPropertyPresets.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class PresetWindow : NonCopyable
	{
	public:
		PresetWindow(Undo::UndoManager& undoManager);
		~PresetWindow() = default;

	public:
		void SyncGui(Chart& chart);
		void SequenceGui(Chart& chart);

		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);

	private:
		Undo::UndoManager& undoManager;

		std::vector<StaticSyncPreset> staticSyncPresets;
		std::optional<DynamicSyncPreset> hoveredDynamicSyncPreset = {};

		const Graphics::SprSet* editorSprites;
		struct SpriteCache
		{
			std::array<const Graphics::Spr*, EnumCount<DynamicSyncPreset>()> DynamicSyncPresetIcons;
		} sprites = {};
	};
}

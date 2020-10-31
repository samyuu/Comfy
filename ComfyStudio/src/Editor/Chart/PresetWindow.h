#pragma once
#include "Types.h"
#include "CoreTypes.h"
#include "Chart.h"
#include "TargetPropertyPresets.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
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

		void OnRenderWindowRender(Chart& chart, TargetRenderWindow& renderWindow, Render::Renderer2D& renderer, TargetRenderHelper& renderHelper);
		void OnRenderWindowOverlayGui(Chart& chart, TargetRenderWindow& renderWindow, ImDrawList& drawList);

		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);

	private:
		void RenderSyncPresetPreview(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, u32 targetCount, const std::array<PresetTargetData, Rules::MaxSyncPairCount>& presetTargets);

	private:
		Undo::UndoManager& undoManager;

		std::vector<StaticSyncPreset> staticSyncPresets;
		
		struct ButtonHoverData
		{
			std::optional<DynamicSyncPreset> DynamicSyncPreset;
			std::optional<size_t> StaticSyncPreset;
		} buttonHover = {};
		
		struct PresetPreviewData
		{
			u32 TargetCount;
			std::array<PresetTargetData, Rules::MaxSyncPairCount> Targets;
		} presetPreview = {};

		const Graphics::SprSet* editorSprites;
		struct SpriteCache
		{
			std::array<const Graphics::Spr*, EnumCount<DynamicSyncPreset>()> DynamicSyncPresetIcons;
		} sprites = {};
	};
}

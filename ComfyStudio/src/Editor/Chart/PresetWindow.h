#pragma once
#include "Types.h"
#include "Chart.h"
#include "TargetPropertyPresets.h"
#include "Editor/Chart/RenderWindow/TargetRenderWindow.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Undo/Undo.h"
#include "ImGui/Gui.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor;

	class PresetWindow : NonCopyable
	{
	public:
		PresetWindow(ChartEditor& chartEditor, Undo::UndoManager& undoManager);
		~PresetWindow() = default;

	public:
		// NOTE: Expected to be called together within a single frame but split into different functions to treat them as seperate windows
		void SyncGui(Chart& chart);
		void SequenceGui(Chart& chart);
		void UpdateStateAfterBothGuiPartsHaveBeenDrawn();

		void OnRenderWindowRender(Chart& chart, TargetRenderWindow& renderWindow, Render::Renderer2D& renderer);
		void OnRenderWindowOverlayGui(Chart& chart, TargetRenderWindow& renderWindow, ImDrawList& drawList);

		void OnEditorSpritesLoaded(const Graphics::SprSet* sprSet);

	private:
		f32 GetPresetPreviewDimness(bool overlayPass) const;
		f32 GetHoverFadeInPreviewOpacity() const;

		void RenderSyncPresetPreview(Render::Renderer2D& renderer, TargetRenderHelper& renderHelper, u32 targetCount, const std::array<PresetTargetData, Rules::MaxSyncPairCount>& presetTargets);

	private:
		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;

		DynamicSyncPresetSettings dynamicSyncPresetSettings = {};
		SequencePresetSettings sequencePresetSettings = {};

		struct HoverData
		{
			struct SyncPresetData
			{
				bool AnyChildWindow;
				bool DynamincChildWindow;
				bool StaticChildWindow;
				bool AddChildWindow;
				bool ContextMenu;
				bool ContextMenuOpen;
				std::optional<DynamicSyncPreset> DynamicPreset;
				std::optional<size_t> StaticPreset;
			} Sync;

			struct SequencePresetData
			{
				// TODO: Future additions, at least to configure the sequencePresetSettings
				bool AnyChildWindow;
				bool ChildWindow;
				std::optional<size_t> Preset;
			} Sequence;

			bool AnyHoveredThisFrame, AnyHoveredLastFrame;
			Stopwatch LastHoverStopwatch;
			Stopwatch HoverDurationStopwatch;
		} hovered = {};

		struct SyncPresetPreviewData
		{
			u32 TargetCount;
			std::array<PresetTargetData, Rules::MaxSyncPairCount> Targets;
		} syncPresetPreview = {};

		const Graphics::SprSet* editorSprites;
		struct SpriteCache
		{
			std::array<const Graphics::Spr*, EnumCount<DynamicSyncPreset>()> DynamicSyncPresetIcons;
		} sprites = {};
	};
}

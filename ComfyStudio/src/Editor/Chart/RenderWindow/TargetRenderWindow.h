#pragma once
#include "Types.h"
#include "Tools/TargetBoxSelectionTool.h"
#include "Tools/TargetTool.h"
#include "TargetRenderHelper.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Window/RenderWindow.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Render/Render.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	constexpr vec2 TargetPositionOrPreset(const TimelineTarget& target)
	{
		return target.Flags.HasProperties ? target.Properties.Position : target.Flags.IsChain ?
			Rules::PresetTargetChainPosition(target.Type, target.Tick, target.Flags) :
			Rules::PresetTargetPosition(target.Type, target.Tick, target.Flags);
	}

	constexpr std::array<u32, EnumCount<ButtonType>()> ButtonTypeColors =
	{
		0xFFCCFE62,
		0xFFD542FF,
		0xFFFEFF62,
		0xFF6412FE,
		0xFF2BD7FF,
		0xFF2BD7FF,
	};

	constexpr u32 GetButtonTypeColorU32(ButtonType type)
	{
		return ButtonTypeColors[static_cast<u8>(type)];
	}

	constexpr u32 GetButtonTypeColorU32(ButtonType type, u8 alpha)
	{
		return (ButtonTypeColors[static_cast<u8>(type)] & 0x00FFFFFF) | (static_cast<u32>(alpha) << 24);
	}
}

namespace Comfy::Studio::Editor
{
	class ChartEditor;
	class TargetTimeline;

	class TargetRenderWindow : public RenderWindow
	{
	public:
		static constexpr f32 TargetHitboxSize = 64.0f;
		static constexpr f32 SelectionCenterMarkerSize = 9.0f;

	public:
		TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager, Render::Renderer2D& renderer);
		~TargetRenderWindow() = default;

	public:
		void SetWorkingChart(Chart* chart);

	public:
		ImTextureID GetTextureID() const override;

	public:
		vec2 TargetAreaToScreenSpace(const vec2 targetAreaSpace) const;
		vec2 ScreenToTargetAreaSpace(const vec2 screenSpace) const;

		const Render::Camera2D& GetCamera() const;

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;

		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	private:
		void UpdateAllInput();
		void UpdateInputContextMenu();

		void SelectActiveTool(TargetToolType toolType);
		TargetTool* GetSelectedTool();

	private:
		void RenderBackground();
		void RenderHUDBackground();
		void RenderAllVisibleTargets();
		void AddVisibleTargetsToDrawBuffers();
		void FlushRenderTargetsDrawBuffers();

	private:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		TargetTimeline& timeline;

		Undo::UndoManager& undoManager;
		Render::Renderer2D& renderer;

	private:
		CheckerboardGrid backgroundCheckerboard;
		f32 backgroundDim = 0.35f;

		struct LayerData
		{
			bool DrawButtons = true;
			bool DrawTargets = true;
			bool DrawTargetHands = true;
		} layers;

		bool drawCheckerboard = true;
		bool drawTargetGrid = true;

		struct PracticeBackgroundData
		{
			bool Enabled = false;
			TargetRenderHelper::BackgroundData Data = {};
		} practiceBackground;

		TimelineTick targetPostHitLingerDuration = TimelineTick::FromBeats(1);

		TargetBoxSelectionTool boxSelectionTool = { *this };

		std::array<std::unique_ptr<TargetTool>, EnumCount<TargetToolType>()> availableTools;
		TargetToolType selectedToolType = TargetToolType::Count;

		Render::Camera2D camera;
		std::unique_ptr<TargetRenderHelper> renderHelper = nullptr;
		std::unique_ptr<Render::RenderTarget2D> renderTarget = nullptr;

		struct DrawBufferData
		{
			std::vector<TargetRenderHelper::TargetData> Targets;
			std::vector<TargetRenderHelper::ButtonData> Buttons;
			std::vector<TargetRenderHelper::ButtonTrailData> Trails;
			std::vector<TargetRenderHelper::ButtonSyncLineData> SyncLines;
			std::vector<std::pair<vec2, u32>> CenterMarkers;
		} drawBuffers;
	};
}

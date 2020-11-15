#pragma once
#include "Types.h"
#include "Tools/TargetBoxSelectionTool.h"
#include "Tools/TargetTool.h"
#include "TargetRenderHelper.h"
#include "TargetGuiDrawUtil.h"
#include "Editor/Chart/Chart.h"
#include "Editor/Chart/TargetPropertyRules.h"
#include "Window/RenderWindow.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Render/Render.h"
#include "Undo/Undo.h"
#include <functional>

namespace Comfy::Studio::Editor
{
	class ChartEditor;
	class TargetTimeline;
	class TargetRenderWindow;

	using TargetRenderWindowRenderCallback = std::function<void(TargetRenderWindow&, Render::Renderer2D&)>;
	using TargetRenderWindowOverlayGuiCallback = std::function<void(TargetRenderWindow&, ImDrawList&)>;

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

		void RegisterRenderCallback(TargetRenderWindowRenderCallback onRender);
		void RegisterOverlayGuiCallback(TargetRenderWindowOverlayGuiCallback onOverlayGui);

	public:
		ImTextureID GetTextureID() const override;

	public:
		vec2 TargetAreaToScreenSpace(const vec2 targetAreaSpace) const;
		vec2 ScreenToTargetAreaSpace(const vec2 screenSpace) const;

		const Render::Camera2D& GetCamera() const;
		TargetRenderHelper& GetRenderHelper();

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;

		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	private:
		void DrawOverlayGui();

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

		TargetRenderWindowRenderCallback onRenderCallback;
		TargetRenderWindowOverlayGuiCallback onOverlayGuiCallback;

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

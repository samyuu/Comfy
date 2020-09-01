#pragma once
#include "Types.h"
#include "TargetRenderHelper.h"
#include "Editor/Chart/Chart.h"
#include "Window/RenderWindow.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Render/Render.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class ChartEditor;
	class TargetTimeline;

	class TargetRenderWindow : public RenderWindow
	{
	public:
		TargetRenderWindow(ChartEditor& parent, TargetTimeline& timeline, Undo::UndoManager& undoManager);
		~TargetRenderWindow() = default;

	public:
		ImTextureID GetTextureID() const override;

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;
		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	private:
		void UpdateAllInput();
		void CursorJumpToNextTarget(int direction);

	private:
		void RenderBackground();
		void RenderHUDBackground();
		void RenderAllVisibleTargets();
		void AddVisibleTargetsToDrawBuffers();
		void FlushRenderTargetsDrawBuffers();

	private:
		vec2 TargetAreaToScreenSpace(const vec2 targetAreaSpace) const;
		vec2 ScreenToTargetAreaSpace(const vec2 screenSpace) const;

	private:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		TargetTimeline& timeline;
		Undo::UndoManager& undoManager;

	private:
		CheckerboardGrid backgroundCheckerboard;
		float backgroundDim = 0.25f;

		Render::Camera2D camera;

		std::unique_ptr<TargetRenderHelper> renderHelper = nullptr;

		struct DrawBufferData
		{
			std::vector<TargetRenderHelper::TargetData> Targets;
			std::vector<TargetRenderHelper::ButtonData> Buttons;
			std::vector<TargetRenderHelper::ButtonTrailData> Trails;
			std::vector<TargetRenderHelper::ButtonSyncLineData> SyncLines;
		} drawBuffers;

		std::unique_ptr<Render::Renderer2D> renderer = nullptr;
		std::unique_ptr<Render::RenderTarget2D> renderTarget = nullptr;
	};
}

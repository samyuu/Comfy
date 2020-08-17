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

	class TargetRenderWindow : public RenderWindow
	{
	public:
		TargetRenderWindow(ChartEditor& parent, Undo::UndoManager& undoManager);
		~TargetRenderWindow() = default;

	public:
		ImTextureID GetTextureID() const override;

	protected:
		ImGuiWindowFlags GetRenderTextureChildWindowFlags() const override;
		void OnFirstFrame() override;
		void PreRenderTextureGui() override;
		void PostRenderTextureGui() override;
		void OnResize(ivec2 newSize) override;
		void OnRender() override;

	protected:
		Chart* workingChart = nullptr;

		ChartEditor& chartEditor;
		Undo::UndoManager& undoManager;

	protected:
		CheckerboardGrid backgroundCheckerboard;
		float backgroundDim = 0.25f;

		const vec2 renderSize = vec2(1920.0f, 1080.0f);
		Render::Camera2D camera;

		std::unique_ptr<TargetRenderHelper> renderHelper = nullptr;

		std::unique_ptr<Render::Renderer2D> renderer = nullptr;
		std::unique_ptr<Render::RenderTarget2D> renderTarget = nullptr;

		void RenderBackground();
		void RenderHUD();
	};
}

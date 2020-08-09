#pragma once
#include "Tools/AetTool.h"
#include "AetRenderPreviewData.h"
#include "ObjectMousePicker.h"
#include "Window/RenderWindow.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Common/CameraController2D.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Render/Render.h"
#include "Undo/Undo.h"

namespace Comfy::Studio::Editor
{
	class AetRenderWindow : public RenderWindow
	{
	public:
		AetRenderWindow(Undo::UndoManager& undoManager, Render::Renderer2D& renderer, AetItemTypePtr& selectedAetItem, AetItemTypePtr& cameraSelectedAetItem, AetRenderPreviewData& previewData);
		~AetRenderWindow() = default;

		void SetIsPlayback(bool value);
		float SetCurrentFrame(float value);

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
		void ToolSelectionHeaderGui();
		void TooltipHeaderGui();

		AetTool* GetCurrentTool();
		void CenterFitCamera();

		void UpdateMousePickControls();

	protected:
		void RenderBackground();
		void RenderAetSet(const Graphics::Aet::AetSet* aetSet);
		void RenderScene(const Graphics::Aet::Scene* scene);
		void RenderComposition(const Graphics::Aet::Composition* comp);
		void RenderLayer(const Graphics::Aet::Layer* layer);
		void RenderVideo(const Graphics::Aet::Video* video);

	protected:
		vec2 GetLayerBoundingSize(const std::shared_ptr<Graphics::Aet::Layer>& layer) const;

	protected:
		bool OnObjRender(const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity);
		bool OnObjMaskRender(const Graphics::Aet::Util::Obj& maskObj, const Graphics::Aet::Util::Obj& obj, vec2 positionOffset, float opacity);

	private:
		// NOTE: Fill the rest of the background
		CheckerboardGrid checkerboardBaseGrid;
		// NOTE: Representing the working aet size
		CheckerboardGrid checkerboardGrid;
		// NOTE: Working aet size
		vec2 aetRegionSize = vec2(1920.0f, 1080.0f);

		// NOTE: To make sure objects won't accidentally be mouse picked / unselected
		bool windowHoveredOnMouseClick = false;

		// NOTE: Store the state of the previous frame to avoid accidental mouse picking on mouse release while interacting with a tool
		bool allowMousePickerInput = false, allowedMousePickerInputLastFrame = false;

		bool isPlayback = false;
		float currentFrame = 0.0f;

		// NOTE: Pointers to the AetEditor owned selection
		struct
		{
			AetItemTypePtr& selectedAetItem;
			AetItemTypePtr& cameraSelectedAetItem;
		};

		Undo::UndoManager& undoManager;

		// NOTE: General rendering
		Render::Renderer2D& renderer;
		std::unique_ptr<Render::RenderTarget2D> renderTarget = nullptr;

		AetRenderPreviewData& previewData;

		// NOTE: To handle mouse inputs when no tool is active
		std::unique_ptr<ObjectMousePicker> mousePicker = nullptr;

		// NOTE: To be filled during rendering and then used for mouse interactions
		Graphics::Aet::Util::ObjCache objectCache;

		// NOTE: The variables that will be edited by the current tool before being turned into commands
		vec2 toolSize = vec2(100.0f, 100.0f);
		Graphics::Transform2D toolTransform = Graphics::Transform2D(vec2(0.0f));

		std::array<std::unique_ptr<AetTool>, AetToolType_Count> tools;
		AetToolType currentToolType = AetToolType_Transform;

		Render::Camera2D camera;
		CameraController2D cameraController;
	};
}

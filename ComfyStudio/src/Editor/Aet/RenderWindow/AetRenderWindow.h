#pragma once
#include "Tools/AetTool.h"
#include "AetRenderPreviewData.h"
#include "ObjectMousePicker.h"
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CameraController2D.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Aet/AetRenderer.h"

namespace Comfy::Editor
{
	class AetRenderWindow : public RenderWindowBase, IMutatingEditorComponent
	{
	public:
		AetRenderWindow(AetCommandManager* commandManager, Graphics::Aet::SpriteGetterFunction* spriteGetter, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem, AetRenderPreviewData* previewData);
		~AetRenderWindow();

		void SetIsPlayback(bool value);
		float SetCurrentFrame(float value);

	protected:
		ImGuiWindowFlags GetChildWinodwFlags() const override;

		void OnDrawGui() override;
		void PostDrawGui() override;
		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(ivec2 size) override;

	protected:
		void DrawToolSelectionHeaderGui();
		void DrawTooltipHeaderGui();

		AetTool* GetCurrentTool();
		void CenterFitCamera();

		void UpdateMousePickControls();

	protected:
		void OnInitialize() override;

		void RenderBackground();
		void RenderAetSet(const Graphics::Aet::AetSet* aetSet);
		void RenderScene(const Graphics::Aet::Scene* scene);
		void RenderComposition(const Graphics::Aet::Composition* comp);
		void RenderLayer(const Graphics::Aet::Layer* layer);
		void RenderVideo(const Graphics::Aet::Video* video);

	protected:
		vec2 GetLayerBoundingSize(const RefPtr<Graphics::Aet::Layer>& layer) const;

	protected:
		bool OnObjRender(const Graphics::Aet::AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity);
		bool OnObjMaskRender(const Graphics::Aet::AetMgr::ObjCache& maskObj, const Graphics::Aet::AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity);

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
			AetItemTypePtr* selectedAetItem = nullptr;
			AetItemTypePtr* cameraSelectedAetItem = nullptr;
		};

		// NOTE: General rendering
		UniquePtr<Graphics::GPU_Renderer2D> renderer = nullptr;
		UniquePtr<Graphics::Aet::AetRenderer> aetRenderer = nullptr;

		AetRenderPreviewData* previewData = nullptr;
		
		// NOTE: To handle mouse inputs when no tool is active
		UniquePtr<ObjectMousePicker> mousePicker = nullptr;;

		// NOTE: To be filled during rendering and then used for mouse interactions
		std::vector<Graphics::Aet::AetMgr::ObjCache> objectCache;

		// NOTE: The variables that will be edited by the current tool before being turned into commands
		vec2 toolSize = vec2(100.0f, 100.0f);
		Graphics::Transform2D toolTransform = Graphics::Transform2D(vec2(0.0f));

		std::array<UniquePtr<AetTool>, AetToolType_Count> tools;
		AetToolType currentToolType = AetToolType_Transform;

		Graphics::OrthographicCamera camera;
		CameraController2D cameraController;
	};
}

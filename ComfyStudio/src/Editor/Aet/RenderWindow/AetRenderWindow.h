#pragma once
#include "Tools/AetTool.h"
#include "ObjectMousePicker.h"
#include "Editor/Aet/IMutatingEditorComponent.h"
#include "Editor/Aet/AetSelection.h"
#include "Editor/Core/RenderWindowBase.h"
#include "Editor/Common/CameraController2D.h"
#include "Editor/Common/CheckerboardGrid.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetRenderer.h"
#include "Graphics/Auth2D/AetMgr.h"

namespace Editor
{
	using namespace Graphics::Auth2D;

	class AetRenderWindow : public RenderWindowBase, IMutatingEditorComponent
	{
	public:
		AetRenderWindow(AetCommandManager* commandManager, SpriteGetterFunction* spriteGetter, AetItemTypePtr* selectedAetItem, AetItemTypePtr* cameraSelectedAetItem);
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
		void RenderAetSet(const AetSet* aetSet);
		void RenderAet(const Aet* aet);
		void RenderAetLayer(const AetLayer* aetLayer);
		void RenderAetObj(const AetObj* aetObj);
		void RenderAetRegion(const AetRegion* aetRegion);

	protected:
		vec2 GetAetObjBoundingSize(const RefPtr<AetObj>& aetObj) const;

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
		UniquePtr<Renderer2D> renderer = nullptr;
		UniquePtr<AetRenderer> aetRenderer = nullptr;

		// NOTE: To handle mouse inputs when no tool is active
		UniquePtr<ObjectMousePicker> mousePicker = nullptr;;

		// NOTE: To be filled during rendering and then used for mouse interactions
		std::vector<AetMgr::ObjCache> objectCache;

		// NOTE: The variables that will be edited by the current tool before being turned into commands
		vec2 toolSize = vec2(100.0f, 100.0f);
		Properties toolProperties = AetMgr::DefaultProperites;

		std::array<UniquePtr<AetTool>, AetToolType_Count> tools;
		AetToolType currentToolType = AetToolType_Hand;

		Graphics::OrthographicCamera camera;
		CameraController2D cameraController;
	};
}
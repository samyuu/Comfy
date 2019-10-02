#pragma once
#include "Tools/AetTool.h"
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
	using namespace FileSystem;
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
		void TrySelectObjectAtPosition(vec2 worldSpace);
		const RefPtr<AetObj>* FindObjectAtPosition(vec2 worldSpace);

	private:
		CheckerboardGrid checkerboardBaseGrid;
		CheckerboardGrid checkerboardGrid;
		vec2 aetRegionSize = vec2(1920.0f, 1080.0f);

		// NOTE: To make sure objects won't accidentally be mouse picked / unselected
		bool windowHoveredOnMouseClick = false;

		// NOTE: To compare with the object on mouse release before selecting the object and prevent accidental selection.
		//		 This object is not guaranteed to stay alive and should only be used for a pointer comparison so don't try to dereference it
		const AetObj* mousePickedObjectOnMouseClick = nullptr;

		bool isPlayback = false;
		float currentFrame = 0.0f;

		struct
		{
			AetItemTypePtr* selectedAetItem;
			AetItemTypePtr* cameraSelectedAetItem;
		};

		UniquePtr<Renderer2D> renderer;
		UniquePtr<AetRenderer> aetRenderer;

		vec2 toolSize = vec2(100.0f, 100.0f);
		Properties toolProperties = { vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), 1.0f };

		Array<UniquePtr<AetTool>, AetToolType_Count> tools;
		AetToolType currentToolType = AetToolType_Hand;

		Vector<AetMgr::ObjCache> objectCache;

		Graphics::OrthographicCamera camera;
		CameraController2D cameraController;
	};
}
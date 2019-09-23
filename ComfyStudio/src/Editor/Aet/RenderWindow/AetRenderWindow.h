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
		void OnResize(int width, int height) override;

	protected:
		void DrawToolGui();
		void DrawTooltipHeaderGui();

		AetTool* GetCurrentTool();
		void CenterFitCamera();

	protected:
		void OnInitialize() override;

		void RenderGrid();
		void RenderAetSet(AetSet* aetSet);
		void RenderAet(Aet* aet);
		void RenderAetLayer(AetLayer* aetLayer);
		void RenderAetObj(AetObj* aetObj);
		void RenderAetRegion(AetRegion* aetRegion);

	protected:
		vec2 GetAetObjBoundingSize(const RefPtr<AetObj>& aetObj) const;

	private:
		CheckerboardGrid checkerboardBaseGrid;
		CheckerboardGrid checkerboardGrid;
		vec2 aetRegionSize = vec2(1920.0f, 1080.0f);

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
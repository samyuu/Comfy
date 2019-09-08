#pragma once
#include "Tools/PickerTool.h"
#include "Tools/HandTool.h"
#include "Tools/TransformTool.h"
#include "Tools/RotationTool.h"
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

	class AetRenderWindow : public RenderWindowBase
	{
	public:
		AetRenderWindow(SpriteGetterFunction* spriteGetter);
		~AetRenderWindow();

		void SetActive(Aet* parent, AetItemTypePtr value);
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
		void DrawAnimationPropertiesGui();

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

	private:
		CheckerboardGrid checkerboardBaseGrid;
		CheckerboardGrid checkerboardGrid;
		vec2 aetRegionSize = vec2(1920.0f, 1080.0f);

		bool isPlayback = false;
		float currentFrame = 0.0f;

		Aet* aet = nullptr;
		AetItemTypePtr active;

		UniquePtr<Renderer2D> renderer;
		UniquePtr<AetRenderer> aetRenderer;

		Array<UniquePtr<AetTool>, AetToolType_Count> tools;
		AetToolType currentToolType;

		Vector<AetMgr::ObjCache> objectCache;

		Graphics::OrthographicCamera camera;
		CameraController2D cameraController;
	};
}
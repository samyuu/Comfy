#pragma once
#include "Editor/Aet/AetSelection.h"
#include "BoxTransformControl.h"
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
		void OnInitialize() override;

		void RenderGrid();
		void RenderAetSet(AetSet* aetSet);
		void RenderAet(Aet* aet);
		void RenderAetLayer(AetLayer* aetLayer);
		void RenderAetObj(AetObj* aetObj);
		void RenderAetRegion(AetRegion* aetRegion);

	protected:
		void RenderObjCache(const AetMgr::ObjCache& obj);
		void RenderObjCache(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj);
		void RenderObjCache(const std::vector<AetMgr::ObjCache>& objectCache);

	private:
		CheckerboardGrid checkerboardGrid;

		bool isPlayback = false;
		float currentFrame = 0.0f;

		Aet* aet = nullptr;
		AetItemTypePtr active;

		UniquePtr<Renderer2D> renderer;
		UniquePtr<AetRenderer> aetRenderer;
		
		const vec4 dummyColor = vec4(0.79f, 0.90f, 0.57f, 0.50f);

		std::vector<AetMgr::ObjCache> objectCache;

		Graphics::OrthographicCamera camera;
		CameraController2D cameraController;

		bool useTextShadow = false;
		int currentBlendItem = static_cast<int>(AetBlendMode::Alpha);
	};
}
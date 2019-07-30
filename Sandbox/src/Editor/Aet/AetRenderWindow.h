#pragma once
#include "Selection.h"
#include "BoxTransformControl.h"
#include "Editor/RenderWindowBase.h"
#include "Editor/CameraController2D.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include "Graphics/Camera.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetMgr.h"
#include "App/Task.h"

namespace Editor
{
	using namespace FileSystem;
	using namespace Auth2D;

	typedef bool (*SpriteGetter)(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite);

	class AetRenderWindow : public RenderWindowBase
	{
	public:
		AetRenderWindow(SpriteGetter spriteGetter);
		~AetRenderWindow();

		void SetActive(Aet* parent, AetItemTypePtr value);
		void SetIsPlayback(bool value);
		float SetCurrentFrame(float value);

	protected:
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

	private:
		std::unique_ptr<App::Task> testTask = nullptr;

		struct
		{
			float GridSize = 1.0f;
			vec4 Color = vec4(0.15f, 0.15f, 0.15f, 1.0f);
			vec4 ColorAlt = vec4(0.32f, 0.32f, 0.32f, 1.0f);
		} gridConfig;

		bool isPlayback = false;
		float currentFrame = 0.0f;

		Aet* aet = nullptr;
		AetItemTypePtr active;

		Renderer2D renderer;
		const vec4 dummyColor = vec4(0.79f, 0.90f, 0.57f, 0.50f);

		std::vector<AetMgr::ObjCache> objectCache;

		OrthographicCamera camera;
		CameraController2D cameraController;

		bool useTextShadow = false;
		int currentBlendItem = static_cast<int>(AetBlendMode::Alpha);

		SpriteGetter getSprite = nullptr;
	};
}
#pragma once
#include "Selection.h"
#include "Editor/RenderWindowBase.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include "Graphics/Auth2D/Renderer2D.h"
#include "Graphics/Auth2D/AetMgr.h"

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
		void UpdateViewMatrix();
		void UpdateViewControlInput();
		
		void SetUpdateCameraZoom(float newZoom, vec2 origin);

		void RenderObjCache(const AetMgr::ObjCache& obj);

	private:
		float currentFrame = 0.0f;

		Aet* aet = nullptr;
		AetItemTypePtr active;

		Renderer2D renderer;

		std::vector<AetMgr::ObjCache> objectCache;

		struct AetCamera
		{
			vec2 Position;
			mat4 ViewMatrix;
			
			const float ZoomStep = 1.1f;
			const float ZoomMin = 0.1f;
			const float ZoomMax = 12.8f;
			float Zoom = 1.0f;
		} camera;

		bool useTextShadow = false;
		int currentBlendItem = static_cast<int>(AetBlendMode::Alpha);

		SpriteGetter getSprite = nullptr;
	};
}
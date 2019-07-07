#pragma once
#include "Selection.h"
#include "Editor/RenderWindowBase.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include "Graphics/Auth2D/Renderer2D.h"

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

		void SetActive(AetLyo* parent, AetItemTypePtr value);

	protected:
		void OnDrawGui() override;
		void OnUpdateInput() override;
		void OnUpdate() override;
		void OnRender() override;
		void OnResize(int width, int height) override;

	protected:
		void OnInitialize() override;

		void RenderAetObj(AetObj* aetObj);
		void RenderAetLayer(AetLayer* aetLayer);
		void RenderAetRegion(AetRegion* aetRegion);

	private:
		AetLyo* aetLyo = nullptr;
		AetItemTypePtr active;

		vec2 newRendererSize;
		Renderer2D renderer;

		struct
		{
			vec4 aetSourceRegion = { 0.0f, 0.0f, 0.0f, 0.0f };
			vec4 aetColor = { 1.0f, 1.0f, 1.0f, 1.0f };
			vec2 aetPosition = { 100, 100 };
			vec2 aetOrigin = { };
			vec2 aetScale = { 1.0f, 1.0f };
			float aetRotation = { };
		};

		bool useTextShadow = false;
		int currentBlendItem = (int)AetBlendMode::Alpha;
		int txpIndex = 0, spriteIndex = -1;

		SpriteGetter getSprite = nullptr;
	};
}
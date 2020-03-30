#pragma once
#include "AetMgr.h"
#include "Graphics/Auth2D/SprSet.h"
#include "Graphics/GPU/GPURenderers.h"
#include <functional>
#include <optional>

namespace Comfy::Graphics::Aet
{
	typedef std::function<bool(const VideoSource* source, const Txp** outTxp, const Spr** outSpr)> SpriteGetterFunction;

	typedef std::function<bool(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)> AetObjCallbackFunction;
	typedef std::function<bool(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)> AetObjMaskCallbackFunction;

	class AetRenderer
	{
	public:
		AetRenderer(GPU_Renderer2D* renderer);
		~AetRenderer() = default;

	public:
		GPU_Renderer2D* GetRenderer2D();
		void SetRenderer2D(GPU_Renderer2D* value);
		
		SpriteGetterFunction* GetSpriteGetterFunction();
		void SetSpriteGetterFunction(SpriteGetterFunction* value);
		
		void SetCallback(const AetObjCallbackFunction& value);
		void SetMaskCallback(const AetObjMaskCallbackFunction& value);

	public:
		static constexpr vec4 DummyColor = vec4(0.79f, 0.90f, 0.57f, 0.50f);

		void RenderObjCache(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity = 1.0f);
		void RenderObjCacheMask(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity = 1.0f);
		
		void RenderObjCacheVector(const std::vector<AetMgr::ObjCache>& objectCache, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);

		void RenderLayer(const Layer* layer, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void RenderLayerLooped(const Layer* layer, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void RenderLayerClamped(const Layer* layer, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
	
		void RenderAetSprite(const Video* video, const VideoSource* source, const vec2& position);

		static bool SpriteNameSprSetSpriteGetter(const SprSet* sprSet, const VideoSource* source, const Txp** outTxp, const Spr** outSpr);

	public:
		bool GetSprite(const VideoSource* source, const Txp** outTxp, const Spr** outSpr);

	private:
		GPU_Renderer2D* renderer2D = nullptr;
		SpriteGetterFunction* spriteGetter = nullptr;

		std::optional<AetObjCallbackFunction> objCallback;
		std::optional<AetObjMaskCallbackFunction> objMaskCallback;

		std::vector<AetMgr::ObjCache> objectCache;
	};
}

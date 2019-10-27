#pragma once
#include "Renderer2D.h"
#include "AetMgr.h"
#include "AetSet.h"
#include "Graphics/SprSet.h"
#include <functional>
#include <optional>

namespace Graphics
{
	typedef std::function<bool(const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr)> SpriteGetterFunction;

	typedef std::function<bool(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)> AetObjCallbackFunction;
	typedef std::function<bool(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)> AetObjMaskCallbackFunction;

	class AetRenderer
	{
	public:
		AetRenderer(Renderer2D* renderer);
		~AetRenderer();

	public:
		Renderer2D* GetRenderer2D();
		void SetRenderer2D(Renderer2D* value);
		
		SpriteGetterFunction* GetSpriteGetterFunction();
		void SetSpriteGetterFunction(SpriteGetterFunction* value);
		
		void SetAetObjCallback(const AetObjCallbackFunction& value);
		void SetAetObjMaskCallback(const AetObjMaskCallbackFunction& value);

	public:
		static constexpr vec4 DummyColor = vec4(0.79f, 0.90f, 0.57f, 0.50f);

		void RenderObjCache(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity = 1.0f);
		void RenderObjCacheMask(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity = 1.0f);
		
		void RenderObjCacheVector(const std::vector<AetMgr::ObjCache>& objectCache, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);

		void RenderAetObj(const AetObj* aetObj, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void RenderAetObjLooped(const AetObj* aetObj, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
		void RenderAetObjClamped(const AetObj* aetObj, float frame, const vec2& position = vec2(0.0f, 0.0f), float opacity = 1.0f);
	
		void RenderAetSprite(const AetRegion* aetRegion, const AetSpriteIdentifier* aetSprite, const vec2& position);

		static bool SpriteNameSprSetSpriteGetter(const SprSet* sprSet, const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr);

	public:
		bool GetSprite(const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr);

	private:
		Renderer2D* renderer2D = nullptr;
		SpriteGetterFunction* spriteGetter = nullptr;

		std::optional<AetObjCallbackFunction> aetObjCallback;
		std::optional<AetObjMaskCallbackFunction> aetObjMaskCallback;

		std::vector<AetMgr::ObjCache> objectCache;
	};
}
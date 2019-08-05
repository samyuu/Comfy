#pragma once
#include "Renderer2D.h"
#include "AetMgr.h"
#include "FileSystem/Format/AetSet.h"
#include "FileSystem/Format/SprSet.h"
#include <functional>

namespace Auth2D
{
	typedef std::function<bool(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite)> SpriteGetterFunction;

	using namespace FileSystem;

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
		
	public:
		void RenderObjCache(const AetMgr::ObjCache& obj, const vec2& position, float opacity = 1.0f);
		void RenderAetObj(const AetObj* aetObj, float frame, const vec2& position = vec2(0, 0), float opacity = 1.0f);
		void RenderAetObjLooped(const AetObj* aetObj, float frame, const vec2& position = vec2(0, 0), float opacity = 1.0f);
		void RenderAetObjClamped(const AetObj* aetObj, float frame, const vec2& position = vec2(0, 0), float opacity = 1.0f);
	
		static bool SpriteNameSprSetSpriteGetter(SprSet* sprSet, AetSprite* inSprite, Texture** outTexture, Sprite** outSprite);

	protected:
		bool GetSprite(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite);

	private:
		Renderer2D* renderer2D = nullptr;
		SpriteGetterFunction* spriteGetter = nullptr;

		std::vector<AetMgr::ObjCache> objectsCache;
	};
}
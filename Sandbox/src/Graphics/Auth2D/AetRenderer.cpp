#include "AetRenderer.h"
#include "Misc/StringHelper.h"

namespace Auth2D
{
	AetRenderer::AetRenderer(Renderer2D* renderer) : renderer2D(renderer)
	{
	}

	AetRenderer::~AetRenderer()
	{
	}

	Renderer2D* AetRenderer::GetRenderer2D()
	{
		return renderer2D;
	}

	void AetRenderer::SetRenderer2D(Renderer2D* value)
	{
		renderer2D = value;
	}

	SpriteGetterFunction* AetRenderer::GetSpriteGetterFunction()
	{
		return spriteGetter;
	}

	void AetRenderer::SetSpriteGetterFunction(SpriteGetterFunction* value)
	{
		spriteGetter = value;
	}

	void AetRenderer::RenderObjCache(const AetMgr::ObjCache& obj, const vec2& position, float opacity)
	{
		if (obj.Region == nullptr || !obj.Visible)
			return;

		// TODO: render texture mask

		Texture* texture;
		Sprite* sprite;
		bool validSprite = GetSprite(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validSprite)
		{
			renderer2D->Draw(
				texture->Texture2D.get(),
				sprite->PixelRegion,
				obj.Properties.Position + position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity * opacity),
				obj.BlendMode);
		}
	}

	void AetRenderer::RenderAetObj(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		objectsCache.clear();

		AetMgr::GetAddObjects(objectsCache, aetObj, frame);

		for (auto& objCache : objectsCache)
			RenderObjCache(objCache, position, opacity);
	}

	void AetRenderer::RenderAetObjLooped(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(aetObj, fmod(frame, aetObj->LoopEnd - 1.0f), position, opacity);
	}

	void AetRenderer::RenderAetObjClamped(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(aetObj, (frame >= aetObj->LoopEnd ? aetObj->LoopEnd : frame), position, opacity);
	}

	bool AetRenderer::SpriteNameSprSetSpriteGetter(SprSet* sprSet, AetSprite* inSprite, Texture** outTexture, Sprite** outSprite)
	{
		if (inSprite == nullptr)
			return false;

		if (inSprite->SpriteCache != nullptr)
		{
		from_sprite_cache:
			*outTexture = sprSet->TxpSet->Textures[inSprite->SpriteCache->TextureIndex].get();
			*outSprite = inSprite->SpriteCache;
			return true;
		}

		for (auto& sprite : sprSet->Sprites)
		{
			if (EndsWith(inSprite->Name, sprite.Name))
			{
				inSprite->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	bool AetRenderer::GetSprite(AetSprite* inSprite, Texture** outTexture, Sprite** outSprite)
	{
		assert(spriteGetter != nullptr);
		return (*spriteGetter)(inSprite, outTexture, outSprite);
	}
}
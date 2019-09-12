#include "AetRenderer.h"
#include "Misc/StringHelper.h"

namespace Graphics::Auth2D
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

		const FileSystem::Texture* texture;
		const FileSystem::Sprite* sprite;
		bool validSprite = GetSprite(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validSprite)
		{
			renderer2D->Draw(
				texture->GraphicsTexture.get(),
				sprite->PixelRegion,
				obj.Properties.Position + position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, obj.Properties.Opacity * opacity),
				obj.BlendMode);
		}
		else
		{
			renderer2D->Draw(
				nullptr,
				vec4(0, 0, obj.Region->Width, obj.Region->Height),
				obj.Properties.Position + position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * obj.Properties.Opacity * opacity),
				obj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheMask(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& position, float opacity)
	{
		if (maskObj.Region == nullptr || obj.Region == nullptr || !obj.Visible)
			return;

		const FileSystem::Texture* maskTexture;
		const FileSystem::Sprite* maskSprite;
		bool validMaskSprite = GetSprite(maskObj.Region->GetSprite(maskObj.SpriteIndex), &maskTexture, &maskSprite);

		const FileSystem::Texture* texture;
		const FileSystem::Sprite* sprite;
		bool validSprite = GetSprite(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validMaskSprite && validSprite)
		{
			assert(maskTexture != texture);

			renderer2D->Draw(
				maskTexture->GraphicsTexture.get(),
				maskSprite->PixelRegion,
				maskObj.Properties.Position,
				maskObj.Properties.Origin,
				maskObj.Properties.Rotation,
				maskObj.Properties.Scale,
				texture->GraphicsTexture.get(),
				sprite->PixelRegion,
				obj.Properties.Position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, maskObj.Properties.Opacity * obj.Properties.Opacity),
				obj.BlendMode);
		}
		else
		{
			renderer2D->Draw(
				nullptr,
				vec4(0, 0, obj.Region->Width, obj.Region->Height),
				obj.Properties.Position + position,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * maskObj.Properties.Opacity * obj.Properties.Opacity * opacity),
				obj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheVector(const Vector<AetMgr::ObjCache>& objectCache, const vec2& position, float opacity)
	{
		bool singleObject = objectCache.size() == 1;

		for (size_t i = 0; i < objectCache.size(); i++)
		{
			const AetMgr::ObjCache& obj = objectCache[i];

			if (obj.UseTextureMask && !singleObject && (i + 1 < objectCache.size()))
			{
				RenderObjCacheMask(objectCache[i + 1], obj, position, opacity);
				i++;
			}
			else
			{
				RenderObjCache(obj, position, opacity);
			}
		}
	}

	void AetRenderer::RenderAetObj(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		objectCache.clear();

		AetMgr::GetAddObjects(objectCache, aetObj, frame);
		RenderObjCacheVector(objectCache, position, opacity);
	}

	void AetRenderer::RenderAetObjLooped(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(aetObj, fmod(frame, aetObj->LoopEnd - 1.0f), position, opacity);
	}

	void AetRenderer::RenderAetObjClamped(const AetObj* aetObj, float frame, const vec2& position, float opacity)
	{
		RenderAetObj(aetObj, (frame >= aetObj->LoopEnd ? aetObj->LoopEnd : frame), position, opacity);
	}

	void AetRenderer::RenderAetSprite(const AetRegion* aetRegion, const AetSprite* aetSprite, const vec2& position)
	{
		const FileSystem::Texture* texture;
		const FileSystem::Sprite* sprite;

		if (aetRegion->SpriteCount() < 1 || !GetSprite(aetSprite, &texture, &sprite))
		{
			renderer2D->Draw(nullptr, vec4(0, 0, aetRegion->Width, aetRegion->Height), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), AetRenderer::DummyColor);
		}
		else
		{
			renderer2D->Draw(texture->GraphicsTexture.get(), sprite->PixelRegion, vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f));
		}
	}

	bool AetRenderer::SpriteNameSprSetSpriteGetter(const SprSet* sprSet, const AetSprite* inSprite, const FileSystem::Texture** outTexture, const Sprite** outSprite)
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
				// NOTE: Temporary solution, check for IDs in the future
				inSprite->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	bool AetRenderer::GetSprite(const AetSprite* inSprite, const FileSystem::Texture** outTexture, const Sprite** outSprite)
	{
		assert(spriteGetter != nullptr);
		return (*spriteGetter)(inSprite, outTexture, outSprite);
	}
}
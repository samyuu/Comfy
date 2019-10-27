#include "AetRenderer.h"
#include "Misc/StringHelper.h"

namespace Graphics
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

	void AetRenderer::SetAetObjCallback(const AetObjCallbackFunction& value)
	{
		aetObjCallback = value;
	}

	void AetRenderer::SetAetObjMaskCallback(const AetObjMaskCallbackFunction& value)
	{
		aetObjMaskCallback = value;
	}

	void AetRenderer::RenderObjCache(const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		if (obj.Region == nullptr || !obj.Visible)
			return;

		if (aetObjCallback.has_value() && aetObjCallback.value()(obj, positionOffset, opacity))
			return;

		const Txp* texture;
		const Spr* sprite;
		const bool validSprite = GetSprite(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		const vec2 finalPosition = obj.Properties.Position + positionOffset;
		const float finalOpacity = obj.Properties.Opacity * opacity;

		if (validSprite)
		{
			renderer2D->Draw(
				texture->GraphicsTexture.get(),
				sprite->PixelRegion,
				finalPosition,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, finalOpacity),
				obj.BlendMode);
		}
		else
		{
			// TODO: Only render optionally
			renderer2D->Draw(
				nullptr,
				vec4(0.0f, 0.0f, obj.Region->Size),
				finalPosition,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * finalOpacity),
				obj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheMask(const AetMgr::ObjCache& maskObj, const AetMgr::ObjCache& obj, const vec2& positionOffset, float opacity)
	{
		if (maskObj.Region == nullptr || obj.Region == nullptr || !obj.Visible)
			return;

		if (aetObjMaskCallback.has_value() && aetObjMaskCallback.value()(maskObj, obj, positionOffset, opacity))
			return;

		const Txp* maskTexture;
		const Spr* maskSprite;
		const bool validMaskSprite = GetSprite(maskObj.Region->GetSprite(maskObj.SpriteIndex), &maskTexture, &maskSprite);

		const Txp* texture;
		const Spr* sprite;
		const bool validSprite = GetSprite(obj.Region->GetSprite(obj.SpriteIndex), &texture, &sprite);

		if (validMaskSprite && validSprite)
		{
			renderer2D->Draw(
				maskTexture->GraphicsTexture.get(),
				maskSprite->PixelRegion,
				maskObj.Properties.Position,
				maskObj.Properties.Origin,
				maskObj.Properties.Rotation,
				maskObj.Properties.Scale,
				texture->GraphicsTexture.get(),
				sprite->PixelRegion,
				obj.Properties.Position + positionOffset,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(1.0f, 1.0f, 1.0f, maskObj.Properties.Opacity * obj.Properties.Opacity * opacity),
				maskObj.BlendMode);
		}
		else
		{
			renderer2D->Draw(
				nullptr,
				vec4(0.0f, 0.0f, obj.Region->Size),
				obj.Properties.Position + positionOffset,
				obj.Properties.Origin,
				obj.Properties.Rotation,
				obj.Properties.Scale,
				vec4(DummyColor.r, DummyColor.g, DummyColor.b, DummyColor.a * maskObj.Properties.Opacity * obj.Properties.Opacity * opacity),
				maskObj.BlendMode);
		}
	}

	void AetRenderer::RenderObjCacheVector(const std::vector<AetMgr::ObjCache>& objectCache, const vec2& position, float opacity)
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

	void AetRenderer::RenderAetSprite(const AetRegion* aetRegion, const AetSpriteIdentifier* aetSprite, const vec2& position)
	{
		const Txp* texture;
		const Spr* sprite;

		if (aetRegion->SpriteCount() < 1 || !GetSprite(aetSprite, &texture, &sprite))
		{
			renderer2D->Draw(nullptr, vec4(0.0f, 0.0f, aetRegion->Size), vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), AetRenderer::DummyColor);
		}
		else
		{
			renderer2D->Draw(texture->GraphicsTexture.get(), sprite->PixelRegion, vec2(0.0f), vec2(0.0f), 0.0f, vec2(1.0f), vec4(1.0f));
		}
	}

	bool AetRenderer::SpriteNameSprSetSpriteGetter(const SprSet* sprSet, const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr)
	{
		if (identifier == nullptr)
			return false;

		if (identifier->SpriteCache != nullptr)
		{
		from_sprite_cache:
			*outTxp = sprSet->TxpSet->Textures[identifier->SpriteCache->TextureIndex].get();
			*outSpr = identifier->SpriteCache;
			return true;
		}

		for (auto& sprite : sprSet->Sprites)
		{
			if (EndsWith(identifier->Name, sprite.Name))
			{
				// TEMP: Temporary solution, check for IDs in the future
				identifier->SpriteCache = &sprite;
				goto from_sprite_cache;
			}
		}

		return false;
	}

	bool AetRenderer::GetSprite(const AetSpriteIdentifier* identifier, const Txp** outTxp, const Spr** outSpr)
	{
		assert(spriteGetter != nullptr);
		return (*spriteGetter)(identifier, outTxp, outSpr);
	}
}
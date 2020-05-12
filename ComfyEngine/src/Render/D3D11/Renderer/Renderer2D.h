#pragma once
#include "Types.h"
#include "../Direct3D.h"
#include "../Buffer/ConstantBuffer.h"
#include "../Buffer/IndexBuffer.h"
#include "../Buffer/VertexBuffer.h"
#include "../Shader/Shader.h"
#include "../State/BlendState.h"
#include "../State/RasterizerState.h"
#include "../State/InputLayout.h"
#include "../Texture/Texture.h"
#include "Detail/SpriteBatchData.h"
#include "Graphics/Camera.h"

namespace Comfy::Render::D3D11
{
	class Renderer2D : NonCopyable
	{
	public:
		static constexpr u32 MaxBatchItemSize = 2048;

	public:
		Renderer2D();
		~Renderer2D() = default;

	public:
		void Begin(const OrthographicCamera& camera);
		void Draw(vec2 position, vec2 size, vec4 color);
		void Draw(vec2 position, vec2 size, const vec4 colors[4]);
		void Draw(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color);

		void Draw(const Texture2D* texture, vec2 position, vec4 color);
		void Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec4 color);

		void Draw(const Texture2D* texture, vec2 position, vec2 origin, float rotation, vec4 color);
		void Draw(const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color, AetBlendMode blendMode = AetBlendMode::Normal);

		void Draw(
			const Texture2D* maskTexture, vec4 maskSourceRegion, vec2 maskPosition, vec2 maskOrigin, float maskRotation, vec2 maskScale,
			const Texture2D* texture, vec4 sourceRegion, vec2 position, vec2 origin, float rotation, vec2 scale, vec4 color,
			AetBlendMode blendMode);

		void DrawLine(vec2 start, vec2 end, vec4 color, float thickness = 1.0f);
		void DrawLine(vec2 start, float angle, float length, vec4 color, float thickness = 1.0f);

		void DrawRectangle(vec2 topLeft, vec2 topRight, vec2 bottomLeft, vec2 bottomRight, vec4 color, float thickness = 1.0f);
		void DrawCheckerboardRectangle(vec2 position, vec2 size, vec2 origin, float rotation, vec2 scale, vec4 color, float precision = 1.0f);

		void End();

	public:
		const SpriteVertices& GetLastVertices() const;
		const OrthographicCamera* GetCamera() const;

		bool GetDrawTextBorder() const;
		void SetDrawTextBorder(bool value);

		u32 GetDrawCallCount() const;

	private:
		void InternalCreateIndexBuffer();
		void InternalCreateVertexBuffer();
		void InternalCreateInputLayout();

		void InternalCreateBatches();

		void InternalFlush();
		void InternalCheckFlushItems();
		
		void InternalSetBlendMode(AetBlendMode blendMode);

		SpriteBatchPair InternalCheckFlushAddItem();
		SpriteBatchPair InternalAddItem();
		
		void InternalClearItems();

		void InternalDraw(const Texture2D* texture, const vec4* sourceRegion, const vec2* position, const vec2* origin, float rotation, const vec2* scale, const vec4* color, AetBlendMode blendMode = AetBlendMode::Normal);
	};
}

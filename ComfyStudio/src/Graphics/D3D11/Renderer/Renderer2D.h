#pragma once
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

namespace Comfy::Graphics::D3D11
{
	class Renderer2D : NonCopyable
	{
	public:
		static constexpr uint32_t MaxBatchItemSize = 2048;

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

		uint32_t GetDrawCallCount() const;

	private:
		struct CameraConstantData
		{
			mat4 ViewProjection;
		};

		struct SpriteConstantData
		{
			TextureFormat Format;
			TextureFormat MaskFormat;

			AetBlendMode BlendMode;
			uint8_t Padding[3];

			int Flags;
			int DrawTextBorder;

			int DrawCheckerboard;
			vec2 CheckerboardSize;
		};

	private:
		bool drawTextBorder = false;
		bool batchSprites = true;

		uint32_t drawCallCount = 0;

		ShaderPair spriteShader;

		DefaultConstantBufferTemplate<CameraConstantData> cameraConstantBuffer = { 0 };
		DynamicConstantBufferTemplate<SpriteConstantData> spriteConstantBuffer = { 0 };

		UniquePtr<StaticIndexBuffer> indexBuffer = nullptr;
		UniquePtr<DynamicVertexBuffer> vertexBuffer = nullptr;
		UniquePtr<InputLayout> inputLayout = nullptr;

		// NOTE: Disable backface culling for negatively scaled sprites
		RasterizerState rasterizerState = { D3D11_FILL_SOLID, D3D11_CULL_NONE };

		// TODO: Once needed the Renderer2D should expose optional wrapped address modes
		TextureSampler defaultTextureSampler = { D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_BORDER };

		/*
		// TODO: Offload checkerboard rendering from shader
		static constexpr std::array<uint32_t, 4> checkerboardTexturePixels = 
		{
			0xFFFFFFFF, 0x00000000, 
			0x00000000, 0xFFFFFFFF,
		};
		
		ImmutableTexture2D checkerboardTexture = { ivec2(2, 2), checkerboardTexturePixels.data(), D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP };
		*/

		struct AetBlendStates
		{
			BlendState Normal = { AetBlendMode::Normal };
			BlendState Add = { AetBlendMode::Add };
			BlendState Multiply = { AetBlendMode::Multiply };
			BlendState LinearDodge = { AetBlendMode::LinearDodge };
			BlendState Overlay = { AetBlendMode::Overlay };
		} aetBlendStates;

		std::vector<SpriteBatch> batches;
		std::vector<SpriteBatchItem> batchItems;
		std::vector<SpriteVertices> vertices;

		const OrthographicCamera* orthographicCamera = nullptr;

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
